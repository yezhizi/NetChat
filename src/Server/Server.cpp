#include "Server.h"

#include "db_access.h"
#include "server_van.h"

namespace ntc {

using CallBack = std::function<void()>;

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
  int pkt_bytes = this->_van->Recv(client_fd, &pkt);
  if (pkt_bytes <= 0) return;

  // 解析包
  int pkt_id = pkt.packetid();
  auto type = static_cast<PacketType>(pkt_id);
  LOG(INFO) << "Server received packet. id : " << pkt_id;

  // call back
  CallBack cb = nullptr;

  // 生成回复包
  Packet pkt_reply;
  switch (type) {
    case PacketType::ServerStatusRequest: {
      // ServerStatusRequest
      ServerStatusResponse response;
      response.set_online(true);
      response.set_registrable(true);

      Server::packToPacket(PacketType::ServerStatusResponse, response,
                           pkt_reply);
      break;
    }
    case PacketType::ServerStatusUpdateRequest: {
      // ServerStatusUpdateResponse
      LOG(INFO) << "Server received ServerStatusUpdateRequest";
      ServerStatusUpdateResponse response;
      response.set_online(true);
      response.set_registrable(true);

      this->packToPacket(PacketType::ServerStatusUpdateResponse, response,
                         pkt_reply);
      break;
    }
    case PacketType::RegisterRequest: {
      // RegisterRequest
      // reply: RegisterResponse
      RegisterRequest request;
      RegisterResponse response;

      pkt.content().UnpackTo(&request);

      auto n = std::move(request.username());
      auto p = std::move(request.rawpassword());

      bool is_success = true;

      if (n.length() < 3 || n.length() > 32) {
        LOG(INFO) << "illegal username" << n;
        is_success = false;
      }

      if (p.length() < 6 || p.length() > 32) {
        LOG(INFO) << "illegal password" << p;
        is_success = false;
      }

      if (is_success) {
        // the username and pass is legal
        User u{0, n, utils::crypto::SHA256(p)};
        is_success = g_db->createUser(u);  // is user creation sccess
      }

      response.set_success(is_success);
      this->packToPacket(PacketType::RegisterResponse, response, pkt_reply);

      break;
    }
    case PacketType::LoginPreRequest: {
      // LoginPreRequest 用户请求登录前的 challenge
      // reply: LoginPreResponse
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
        response.set_id(-1);
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
        std::string token = utils::crypto::genToken();
        response.set_logined(true);
        response.set_token(token);
        response.set_id(user.getId());
        //  设置用户信息
        UM::Get()->setUserIdByToken(token, user.getId());

      } else {
        // 登录失败
        response.set_logined(false);
        response.set_token("wrong password");
        response.set_id(-1);
      }

      // 写入回复包
      this->packToPacket(PacketType::LoginResponse, response, pkt_reply);
      LOG(INFO) << "Server sent LoginResponse"
                << " logined: " << response.logined()
                << " token: " << response.token() << " id: " << response.id();

      // 删除记录的 challenge
      UM::Get()->delChallengeMp(user.getId());

      break;
    }
    case PacketType::SetupChannelRequest: {
      // SetupChannelResponse
      SetupChannelRequest request;
      pkt.content().UnpackTo(&request);

      // 获取用户信息 token->userId->fd
      auto uid = UM::Get()->getUserIdByToken(request.token());

      LOG(INFO) << "Server received SetupChannelRequest"
                << " token: " << request.token() << " uid: " << uid;

      // ServerAckResponse
      ServerAckResponse response;
      LOG(INFO) << "Server sent ServerAckResponse";
      this->packToPacket(PacketType::ServerAckResponse, response, pkt_reply);

      

      cb = [&]() {
        Packet new_req;
        // 2. 设置长连接 uid->sender+fd  uid->token
        UM::Get()->setKpAliveSender(uid, client_fd);

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
          // TODO:internalIds: the max internal id of the contact
          //  contact_list_request.add_internalids(0);
          //  int max_internal_id = g_db->getMaxInternalId(uid, u.getId());
          int max_internal_id = g_db->getAllMessages(uid, u.getId()).size();
          contact_list_request.add_internalids(max_internal_id);
        }

        this->packToPacket(PacketType::ContactListRequest, contact_list_request,
                           new_req);
        KeepAliveMsgSender *sender = UM::Get()->getSender(uid);

        // 添加到发送队列
        if (sender) {
          sender->addSendTask(new_req);
        } else {
          LOG(ERROR) << "Sender is nullptr";
        }
        LOG(INFO) << "Server pre sent ContactListRequest";
        //  1. 通知van取消对应的监听事件
        this->_van->Control(client_fd, "EPOLL_DEL_FD");
      };
      LOG_IF(!cb, FATAL) << "Callback is nullptr";

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
        response.mutable_message()->set_id(-1);
        this->packToPacket(PacketType::SendMessageResponse, response,
                           pkt_reply);
        break;
      }

      // check receiver_id exists
      auto receiver = g_db->getUser(receiver_id);
      if (!receiver.has_value()) {
        // receiver not exists
        LOG(INFO) << "Receiver not exists. receiver id: " << receiver_id;
        response.mutable_message()->set_id(-1);
        this->packToPacket(PacketType::SendMessageResponse, response,
                           pkt_reply);
        break;
      }

      // create a new message
      auto msg_result = g_db->createMsgByRawMsg(message);
      if (!msg_result.has_value()) {
        // create message failed
        LOG(INFO) << "Create message failed. sender id: " << sender_id
                  << " receiver id: " << receiver_id;
        response.mutable_message()->set_id(-1);
        this->packToPacket(PacketType::SendMessageResponse, response,
                           pkt_reply);
        break;
      }
      auto reply_msg = msg_result.value();
      LOG(DEBUG) << reply_msg.DebugString();

      // check msg type, if IMAGE/FILE, register it
      if (message.type() == netdesign2::MessageType::IMAGE ||
          message.type() == netdesign2::MessageType::FILE) {
        // register a new file
        auto file_id_result = g_db->createFile();
        if (!file_id_result.has_value()) {
          LOG(INFO) << "Register file in db failed, sender id: " << sender_id
                    << " receiver id: " << receiver_id;
          response.mutable_message()->set_id(-1);
          this->packToPacket(PacketType::SendMessageResponse, response,
                             pkt_reply);
          break;
        }

        // update the message with generated file id
        reply_msg.mutable_message()->set_content(file_id_result.value());
      } else {
        // TODO: notify receiver
        cb = [&, reply_msg]() {
          // implement here...
          // 是否在线
          bool is_online = UM::Get()->isOnline(receiver_id);
          if (!is_online) {
            LOG(INFO) << "Receiver is offline. receiver id: " << receiver_id;
            return;
          }

          Packet new_req;
          ContactMessageListRequest contact_message_list_request;
          contact_message_list_request.add_messages()->CopyFrom(reply_msg);

          this->packToPacket(PacketType::ContactMessageListRequest,
                             contact_message_list_request, new_req);
          KeepAliveMsgSender *sender = UM::Get()->getSender(receiver_id);

          // 添加到发送队列
          if (sender) {
            sender->addSendTask(new_req);
          } else {
            LOG(ERROR) << "Sender is nullptr";
          }
          LOG(INFO) << "Server pre sent ContactMessageListRequest";
        };
      }

      // respond to sender
      response.mutable_message()->CopyFrom(reply_msg);
      this->packToPacket(PacketType::SendMessageResponse, response, pkt_reply);
      LOG(INFO) << "Server sent SendMessageResponse";
      break;
    }
    case PacketType::FileUploadRequest: {
      // User request to upload file
      // reply: FileUploadResponse
      FileUploadRequest request;
      FileUploadResponse response;
      pkt.content().UnpackTo(&request);

      // 获取用户信息 token->uid
      auto token = request.token();
      auto uid = UM::Get()->getUserIdByToken(token);
      if (!uid) {
        // token error, decline the request
        LOG(INFO) << "Unmatched userId and token. user id: " << uid;
        response.set_success(false);
        this->packToPacket(PacketType::FileUploadResponse, response, pkt_reply);
        break;
      }

      // check file exists in table
      auto result = g_db->getFile(request.id());
      if (!result.has_value()) {
        // file not registered
        LOG(INFO) << "File not registered. file id: " << request.id();
        response.set_success(false);
        this->packToPacket(PacketType::FileUploadResponse, response, pkt_reply);
        break;
      }

      // Write to disk
      bool success = false;
      auto file = result.value();
      file.setHash(request.hash());
      file.setName(request.file().name());
      if (!file.checkDiskExistence()) {
        success = file.saveToDisk(request.file());
      }

      // update the file in table
      if (!g_db->updateFile(file)) {
        // update failed
        LOG(INFO) << "Update file failed. file id: " << request.id();
        response.set_success(false);
        this->packToPacket(PacketType::FileUploadResponse, response, pkt_reply);
        break;
      }
      LOG (DEBUG) << "Update file success. file id: " << request.id();

      LOG (DEBUG) << "FileUploadResponse success: " << success;
      response.set_success(success);
      this->packToPacket(PacketType::FileUploadResponse, response, pkt_reply);
      break;
    }
    case PacketType::FileDownloadRequest: {
      // User request to download file
      // reply: FileDownloadResponse
      FileDownloadRequest request;
      FileDownloadResponse response;
      pkt.content().UnpackTo(&request);

      // 获取用户信息 token->uid
      auto token = request.token();
      auto uid = UM::Get()->getUserIdByToken(token);
      if (!uid) {
        // token error, decline the request
        LOG(INFO) << "Unmatched userId and token. user id: " << uid;
        response.mutable_file()->set_name("");
        this->packToPacket(PacketType::FileDownloadResponse, response,
                           pkt_reply);
        break;
      }

      // query the file in table
      auto result = g_db->getFile(request.id());
      if (!result.has_value()) {
        // file not exist
        LOG(INFO) << "File not exist. file id: " << request.id();
        response.mutable_file()->set_name("");
        this->packToPacket(PacketType::FileDownloadResponse, response,
                           pkt_reply);
        break;
      }

      // load from disk
      auto file = result.value();
      response.set_hash(file.getHash());
      if (!(file.checkDiskExistence() &&
          file.loadIntoProto(*response.mutable_file()))) {
        LOG(INFO) << "Failed to load file on disk. file id: " << request.id();
        response.mutable_file()->set_name("");
        this->packToPacket(PacketType::FileDownloadResponse, response,
                           pkt_reply);
        break;
      };

      // respond to sender
      this->packToPacket(PacketType::FileDownloadResponse, response, pkt_reply);
      break;
    }
    case PacketType::ContactMessageRequest: {
      // 用户拉取最新的消息
      // reply: ContactMessageResponse
      ContactMessageRequest request;
      ContactMessageResponse response;
      pkt.content().UnpackTo(&request);

      auto token = request.token();
      auto uid = UM::Get()->getUserIdByToken(token);

      auto result = g_db->getMessage(uid, request.id(), request.internalid());
      if (!result.has_value()) {
        LOG(WARNING) << "Get message failed. user id: " << uid
                     << " contact id: " << request.id()
                     << " internal id: " << request.internalid();
        response.mutable_message()->set_id(-1);
        this->packToPacket(PacketType::ContactMessageResponse, response,
                           pkt_reply);
        break;
      }
      LOG (DEBUG) << "Get message success. user id: " << uid
                     << " contact id: " << request.id()
                     << " internal id: " << request.internalid();

      response.mutable_message()->CopyFrom(result.value());
      this->packToPacket(PacketType::ContactMessageResponse, response,
                         pkt_reply);
      break;
    }
    case PacketType::DeleteMessageRequest: {
      // Ignored
      LOG(WARNING) << "DeleteMsgReq is ignored";
      return;
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
    return;
  }
  // run call back
  if (static_cast<bool>(cb)) {
    LOG(DEBUG) << "run callback";
    cb();
  }
  LOG(INFO) << "Server sent packet. id : " << pkt_reply.packetid();
}

/////////// KeepAliveMsgSender////////////
void Server::KeepAliveMsgSender::run() {
  // 从队列中取出消息并发送
  while (true) {
    Packet pkt;
    this->msg_queue_.WaitAndPop(&pkt);
    if (pkt.packetid() == static_cast<int>(PacketType::ContactListRequest)) {
      std::this_thread::sleep_for(std::chrono::seconds(1));
      LOG(INFO) << "Channel: Send ContactListRequest";
    } else if (pkt.packetid() ==
               static_cast<int>(PacketType::ContactMessageListRequest)) {
      LOG(INFO) << "Channel: Send ContactMessageListRequest";
    }
    std::lock_guard<std::mutex> lock(this->mu_);
    {
      int ret = Server::Get()._van->Send(pkt, this->fd_);
      if (ret < 0) {
        // 发送失败
        // 关闭socket
        LOG(INFO) << "KeepAliveMsgSender send failed. fd: " << this->fd_;
        Server::Get()._van->Control(this->fd_, "CLOSE_FD");
        // break;
      }
      // TODO: 接收有问题！！暂时先测试接收是否有问题
      std::this_thread::sleep_for(std::chrono::seconds(3));
      // 接收ack
      Packet ack;
      int pkt_bytes = Server::Get()._van->Recv(this->fd_, &ack);
      if (pkt_bytes < 0) {
        // 接收失败
        // 关闭socket
        LOG(INFO) << "KeepAliveMsgSender recv failed. fd: " << this->fd_;
        Server::Get()._van->Control(this->fd_, "CLOSE_FD");
        // break;
      }
      // 解析包 ClientAckResponse
      int pkt_id = ack.packetid();
      auto type = static_cast<PacketType>(pkt_id);
      if (type != PacketType::ClientAckResponse) {
        // 接收到的包不是ClientAckResponse
        // 关闭socket
        LOG(INFO) << "KeepAliveMsgSender recv wrong packet. fd: " << this->fd_;

        // break;
      }
      LOG(DEBUG) << "KeepAliveChannel: recv ack from client";
    }
  }
  return;
}

}  // namespace ntc