#include "Server.h"

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
void Server::processRevcSocket(const int client_fd) const {
  // TODO
  Packet msg;
  this->_van->Recv(client_fd, &msg);
  int packet_id = msg.packetid();
  MessageType type = static_cast<MessageType>(packet_id);
  LOG(INFO) << "Server received packet. id : " << packet_id;
  Packet packet_back;
  switch (type) {
    case MessageType::ServerStatusRequest: {
      // ServerStatusRequest
      ServerStatusResponse response;
      response.set_online(true);
      response.set_registrable(false);

      Server::packtoPacket(MessageType::ServerStatusResponse, response,
                           packet_back);
      break;
    }
    case MessageType::ServerStatusUpdateRequest: {
      // ServerStatusUpdateResponse
      LOG(INFO) << "Server received ServerStatusUpdateRequest";
      ServerStatusUpdateResponse response;
      response.set_online(true);
      response.set_registrable(false);

      this->packtoPacket(MessageType::ServerStatusUpdateResponse, response,
                         packet_back);
      break;
    }
    case MessageType::LoginPreRequest: {
      // LoginResponse
      LoginPreRequest request;
      msg.content().UnpackTo(&request);
      std::string username = request.username();
      LOG(INFO) << "Server received LoginPreRequest"
                << " username: " << username;

      LoginPreResponse response;
      std::string challenge = ntc::genChallenge();
      bool ret = UM::Get()->setChallengeMp(username, challenge);
      if (!ret) {
        // 发现用户未注册
        LOG(INFO) << "User not registered. username: " << username;
        response.set_challenge("");
      } else {
        response.set_challenge(challenge);
      }
      this->packtoPacket(MessageType::LoginPreResponse, response, packet_back);
      LOG(INFO) << "Server sent LoginPreResponse";
      break;
    }
    case MessageType::LoginRequest: {
      // LoginResponse
      LoginRequest request;
      LoginResponse response;

      msg.content().UnpackTo(&request);

      // 判断用户是否存在
      std::string username = request.username();
      std::string challenge = UM::Get()->getChallengeMp(username);
      auto result = g_db->getUser(username);
      if (!result.has_value() || challenge == "") {
        LOG(INFO) << "User not found. username: " << username;
        response.set_logined(false);
        response.set_token("illegal user");
        this->packtoPacket(MessageType::LoginResponse, response, packet_back);
        break;
      }

      // 获取 user
      auto user = result.value();

      // 计算(pass.sha256+challenge).sha256
      std::string hashPassword = request.hashpassword();
      std::string pass_challenge_sha256 =
          ntc::SHA256(user.getPassword() + challenge);
      // 比较
      if (pass_challenge_sha256 == hashPassword) {
        // 登录成功
        // 重复登录问题
        response.set_logined(true);
        std::string token = ntc::genToken();
        response.set_token(token);
        //  设置用户信息
        UM::Get()->setKeepaliveSocketMp(username, client_fd, token);

      } else {
        // 登录失败
        response.set_logined(false);
        response.set_token("wrong password");
      }
      this->packtoPacket(MessageType::LoginResponse, response, packet_back);
      LOG(INFO) << "Server sent LoginResponse"
                << " logined: " << response.logined()
                << " token: " << response.token();
      UM::Get()->delChallengeMp(username);  // 删除记录的challenge
      break;
    }
    case MessageType::SetupChannelRequest: {
      // SetupChannelResponse
      SetupChannelRequest request;
      msg.content().UnpackTo(&request);
      LOG(INFO) << "Server received SetupChannelRequest"
                << " token: " << request.token();
      // 获取用户信息 token->username->fd
      std::string username = UM::Get()->getUsernameByToken(request.token());
      int fd = UM::Get()->getfdByUsername(username);
      if (fd == -1 || fd != client_fd) {
        // user not login or token error
        LOG(INFO) << "User not login or token error. username: " << username;
        this->_van->Control(client_fd, "EPOLL_DEL_FD");
        this->_van->Control(client_fd, "CLOSE_FD");
        break;
      }
      //  1. 通知van取消对应的监听事件
      this->_van->Control(client_fd, "EPOLL_DEL_FD");
      // ServerAckResponse
      ServerAckResponse response;
      LOG(INFO) << "Server sent ServerAckResponse";
      this->packtoPacket(MessageType::ServerAckResponse, response, packet_back);
      // 2. 唤醒在长连接处的工作线程,给客户端发送一些消息(联系人...)
      UM::Get()->setKeepaliveSocketMp(username, client_fd, request.token());
      // 发送联系人列表
      ContactListRequest contact_list_request;
      // TODO: 获取联系人列表
      Contact contact;
      contact.set_id(111);
      contact.set_name("test");
      contact.set_online(true);
      contact.set_type(Contact::ContactType::Contact_ContactType_FRIEND);
      contact_list_request.add_contacts()->CopyFrom(contact);

      this->packtoPacket(MessageType::ContactListRequest, contact_list_request,
                         packet_back);
      this->_van->addSendTask(client_fd, packet_back);

      break;
    }
  }
  if (!packet_back.has_content()) {
    LOG(INFO) << "Unknown message";
    return;
  }

  int ret = this->_van->Send(packet_back, client_fd);

  if (ret < 0) {
    // 发送失败
    // 让server van epoll_ctl删除对应的监听事件,并关闭socket
    this->_van->Control(client_fd, "EPOLL_DEL_FD");
    this->_van->Control(client_fd, "CLOSE_FD");
  }
  LOG(INFO) << "Server sent packet. id : " << packet_back.packetid();
}
}  // namespace ntc