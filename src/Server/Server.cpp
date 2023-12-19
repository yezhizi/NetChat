#include "Server.h"

#include "db_access.h"
#include "server_van.h"

namespace ntc {

void Server::Init() {
  LOG(INFO) << "Server initialized";
  this->_van = (Van *)new ServerVan();
  // init UM
  UM::Get()->BindServer(this);
}

void Server::Finalize() {
  delete this->_van;
  LOG(INFO) << "Server closed";
}

// 收到临时连接池的请求
void Server::processRecvSocket(const int client_fd) const {
  using namespace netdesign2;
  // TODO
  Packet pkt;
  this->_van->Recv(client_fd, &pkt);

  // 解析包
  int pkt_id = pkt.packetid();
  auto type = static_cast<PacketType>(pkt_id);
  LOG(INFO) << "Server received packet. id : " << pkt_id;

  // 生成回复包
  Packet pkt_reply;
  switch (type) {
    case PacketType::ServerStatusRequest: {
      // ServerStatusRequest
      ServerStatusResponse response;
      response.set_online(true);
      response.set_registrable(false);

      Server::packToPacket(PacketType::ServerStatusResponse, response,
                           pkt_reply);
      break;
    }
    case PacketType::ServerStatusUpdateRequest: {
      // ServerStatusUpdateResponse
      LOG(INFO) << "Server received ServerStatusUpdateRequest";
      ServerStatusUpdateResponse response;
      response.set_online(true);
      response.set_registrable(false);

      this->packToPacket(PacketType::ServerStatusUpdateResponse, response,
                         pkt_reply);
      break;
    }
    case PacketType::LoginPreRequest: {
      // LoginPreRequest 用户请求登录前的 challenge
      // reply LoginPreResponse
      LoginPreRequest request;
      LoginPreResponse response;

      pkt.content().UnpackTo(&request);
      std::string username = request.username();
      LOG(INFO) << "Server received LoginPreRequest"
                << " username: " << username;

      // 判断用户是否存在
      auto result = g_db->getUser(username);
      if (!result.has_value()) {
        // User 不存在
        LOG(INFO) << "User not registered. username: " << username;
        response.set_challenge("");
      } else {
        // User 存在
        auto user = std::move(result.value());

        // 生成 challenge
        std::string challenge = utils::crypto::genChallenge();
        UM::Get()->setChallengeMp(user.getId(), challenge);
        response.set_challenge(challenge);
      }

      // 写入回复包
      this->packToPacket(PacketType::LoginPreResponse, response, pkt_reply);
      LOG(INFO) << "Server sent LoginPreResponse";

      break;
    }
    case PacketType::LoginRequest: {
      // LoginRequest 用户经过 challenge 计算后发送的登录请求
      // reply LoginResponse
      LoginRequest request;
      LoginResponse response;

      pkt.content().UnpackTo(&request);

      // 判断用户是否存在
      std::string username = request.username();
      auto result = g_db->getUser(username);
      if (!result.has_value()) {
        LOG(INFO) << "User not found. username: " << username;
        response.set_logined(false);
        response.set_token("illegal user");
        this->packToPacket(PacketType::LoginResponse, response, pkt_reply);
        break;
      }

      // 获取 challenge
      auto user = std::move(result.value());
      std::string challenge = UM::Get()->getChallengeMp(user.getId());

      // 计算(pass.sha256+challenge).sha256
      std::string hashPassword = request.hashpassword();
      std::string pass_challenge_sha256 =
          utils::crypto::SHA256(user.getPassword() + challenge);

      // 比较
      if (pass_challenge_sha256 == hashPassword) {
        // 登录成功
        // TODO: 重复登录问题 ?
        std::string token = utils::crypto::genToken();
        response.set_logined(true);
        response.set_token(token);
        //  设置用户信息
        UM::Get()->setKeepaliveSocketMp(user.getId(), client_fd, token);
      } else {
        // 登录失败
        response.set_logined(false);
        response.set_token("wrong password");
      }

      // 写入回复包
      this->packToPacket(PacketType::LoginResponse, response, pkt_reply);
      LOG(INFO) << "Server sent LoginResponse"
                << " logined: " << response.logined()
                << " token: " << response.token();

      // 删除记录的 challenge
      UM::Get()->delChallengeMp(user.getId());

      break;
    }
    case PacketType::SetupChannelRequest: {
      // SetupChannelResponse
      SetupChannelRequest request;
      pkt.content().UnpackTo(&request);
      LOG(INFO) << "Server received SetupChannelRequest"
                << " token: " << request.token();

      // 获取用户信息 token->userId->fd
      auto uid = UM::Get()->getUserIdByToken(request.token());
      int fd = UM::Get()->getfdByUserId(uid);
      if (fd == -1 || fd != client_fd) {
        // user not login or token error
        LOG(INFO) << "User not login or token error. user id: " << uid;
        this->_van->Control(client_fd, "EPOLL_DEL_FD");
        this->_van->Control(client_fd, "CLOSE_FD");
        break;
      }

      //  1. 通知van取消对应的监听事件
      this->_van->Control(client_fd, "EPOLL_DEL_FD");
      // ServerAckResponse
      ServerAckResponse response;
      LOG(INFO) << "Server sent ServerAckResponse";
      this->packToPacket(PacketType::ServerAckResponse, response, pkt_reply);

      // 2. 唤醒在长连接处的工作线程,给客户端发送一些消息(联系人...)
      UM::Get()->setKeepaliveSocketMp(uid, client_fd, request.token());

      // 发送联系人列表
      ContactListRequest contact_list_request;

      // 获取联系人列表
      auto users = g_db->getAllUsers();
      for (auto u : users) {
        Contact contact;
        contact.set_id(u.getId());
        contact.set_name(u.getUsername());
        contact.set_online(true);
        contact.set_type(Contact::ContactType::Contact_ContactType_FRIEND);

        contact_list_request.add_contacts()->CopyFrom(contact);
      }

      this->packToPacket(PacketType::ContactListRequest, contact_list_request,
                         pkt_reply);
      this->_van->addSendTask(client_fd, pkt_reply);

      break;
    }
    case PacketType::SendMessageRequest: {
      // User request to send message
      // reply: SendMessageResponse
      SendMessageRequest request;
      SendMessageResponse response;
      pkt.content().UnpackTo(&request);

      // 获取用户信息 token->uid
      auto token = request.token();
      auto uid = UM::Get()->getUserIdByToken(token);

      // 获取消息信息
      auto message = request.message();
      auto sender_id = message.from();
      auto receiver_id = message.to();

      if (uid != sender_id) {
        // token error, ignore
        LOG(INFO) << "Unmatched userId and token. user id: " << uid;
        break;
      }

      // check receiver_id exists
      auto receiver = g_db->getUser(receiver_id);
      if (!receiver.has_value()) {
        // receiver not exists
        LOG(INFO) << "Receiver not exists. receiver id: " << receiver_id;
        break;
      }

      // save message to db
      // auto result = g_db->createSavedMessage(message);


      break;
    }
    default:  // 未知消息
      LOG(INFO) << "Unknown message";
      return;
  }

  if (!pkt_reply.has_content()) {
    LOG(INFO) << "Unknown message";
    return;
  }

  int ret = this->_van->Send(pkt_reply, client_fd);

  if (ret < 0) {
    // 发送失败
    // 让server van epoll_ctl删除对应的监听事件,并关闭socket
    this->_van->Control(client_fd, "EPOLL_DEL_FD");
    this->_van->Control(client_fd, "CLOSE_FD");
  }
  LOG(INFO) << "Server sent packet. id : " << pkt_reply.packetid();
}
}  // namespace ntc