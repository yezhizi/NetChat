#ifndef SERVER_H
#define SERVER_H

#include <sys/socket.h>

#include <mutex>
#include <unordered_map>

#include "crypto_utils.h"
#include "logging.h"
#include "message.h"
#include "van.h"

// libsodium init
namespace ntc {

class Server {
  friend class UM;

 private:
  Van *_van;
  // 临时连接池的映射  ipport -> fd
  std::unordered_map<std::string, int> _revc_socket_pool;

  // 登录时challenge的映射  username -> challenge
  std::unordered_map<std::string, std::string> _challenge_pool;

  // 保活连接池映射 username -> fd
  std::unordered_map<std::string, int> _keepalive_socket_pool;
  // token -> username
  std::unordered_map<std::string, std::string> _token_pool;

  void Init();
  void Finalize();

  Server() { this->Init(); }
  Server(const Server &) = delete;
  Server &operator=(const Server &) = delete;
  // 将具体的消息打包成packet
  static void packtoPacket(PacketType ptype, google::protobuf::Message &content,
                           Packet &packet) {
    packet.set_packetid(static_cast<int>(ptype));
    google::protobuf::Any *content_ = packet.mutable_content();
    content_->PackFrom(content);
  }

 public:
  ~Server() { this->Finalize(); }

  static Server &Get() {
    static Server server;
    return server;
  }
  const Van *getVan() const { return this->_van; }
  static inline int getRevcSocketNum() { return revc_socket_pool_thread_num; }
  static inline int getKeepAliveNum() { return keepalive_thread_num; }

  void processRevcSocket(const int client_fd) const;
};

}  // namespace ntc

#endif