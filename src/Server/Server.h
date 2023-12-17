#ifndef SERVER_H
#define SERVER_H

#include <sys/socket.h>

#include <mutex>
#include <unordered_map>

#include "base.h"
#include "message.h"
#include "utils/crypto.h"
#include "utils/logging.h"
#include "van.h"

namespace ntc {

class Server {
  using Packet = netdesign2::Packet;

  friend class UM;

 private:
  Van *_van;
  // 临时连接池的映射  ipport -> fd
  std::unordered_map<std::string, int> _revc_socket_pool;

  // 登录时challenge的映射  userid -> challenge
  std::unordered_map<int, std::string> _challenge_pool;

  // 保活连接池映射 userid -> fd
  std::unordered_map<int, int> _keepalive_socket_pool;

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