#ifndef SERVER_H
#define SERVER_H

#include <sys/socket.h>

#include <mutex>
#include <unordered_map>

#include "base.h"
#include "message.h"
#include "threadsafe_queue.h"
#include "utils/crypto.h"
#include "utils/logging.h"
#include "van.h"

namespace ntc {

class Server {
  using Packet = netdesign2::Packet;

  friend class UM;
  class KeepAliveMsgSender;

 private:
  Van *_van;
  // 临时连接池的映射  ipport -> fd
  std::unordered_map<std::string, int> _revc_socket_pool;

  // 登录时challenge的映射  userid -> challenge
  std::unordered_map<int, std::string> _challenge_pool;

  // 保活连接池映射 userid -> sender
  std::unordered_map<int, std::unique_ptr<KeepAliveMsgSender>>
      _keepalive_sender_mp;

  // token -> userid
  std::unordered_map<std::string, int> _token_pool;

  void Init();
  void Finalize();

  Server() { this->Init(); }
  Server(const Server &) = delete;
  Server &operator=(const Server &) = delete;

  // 将具体的消息打包成packet
  static void packToPacket(PacketType ptype, google::protobuf::Message &content,
                           Packet &packet) {
    packet.set_packetid(static_cast<int>(ptype));
    google::protobuf::Any *content_ = packet.mutable_content();
    content_->PackFrom(content);
  }

  // KeepAliveMsgSender内部类
  class KeepAliveMsgSender {
   public:
    KeepAliveMsgSender(const int uid, const int fd) : fd_(fd), uid_(uid) {
      keepalive_msg_sender_thread_ =
          std::thread(&KeepAliveMsgSender::run, this);
    }
    inline int getFd() const {
      std::lock_guard<std::mutex> lock(this->mu_);
      return this->fd_;
    }
    inline void setFd(const int fd) {
      std::lock_guard<std::mutex> lock(this->mu_);
      this->fd_ = fd;
    }
    inline void addSendTask(const Packet &packet) {
      this->msg_queue_.Push(packet);
    }
    ~KeepAliveMsgSender() { this->keepalive_msg_sender_thread_.join(); }

   private:
    void run();
    std::thread keepalive_msg_sender_thread_;
    // msg queue
    ThreadsafeQueue<Packet> msg_queue_;
    // fd
    int fd_;
    int uid_;
    mutable std::mutex mu_;
  };

 public:
  ~Server() { this->Finalize(); }

  static Server &Get() {
    static Server server;
    return server;
  }
  const Van *getVan() const { return this->_van; }
  static inline int getRecvSocketNum() { return revc_socket_pool_thread_num; }
  static inline int getKeepAliveNum() { return keepalive_thread_num; }

  void processRecvSocket(const int client_fd) const;
};

}  // namespace ntc

#endif