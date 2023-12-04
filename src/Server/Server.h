#pragma once
#include "logging.h"
#include "van.h"

#include "message.h"
#include <mutex>
#include <sys/socket.h>
#include <unordered_map>
namespace ntc {

class Server {
    friend class UM;

  private:
    Van *_van;

    void Init();
    void Finalize();

    Server() { this->Init(); }
    Server(const Server &) = delete;
    Server &operator=(const Server &) = delete;

    // 临时连接池的映射  ipport -> fd
    std::unordered_map<std::string, int> _revc_socket_pool;

    // 保活连接池映射 userid -> fd
    std::unordered_map<std::string, int> _keepalive_socket_pool;

  public:
    static Server *Get() {
        static Server server;
        return &server;
    }
    const Van *getVan() const { return this->_van; }
    static inline int getRevcSocketNum() { return revc_socket_pool_thread_num; }

    // 将具体的消息打包成packet
    void packtoPacket(MessageType messagetype,google::protobuf::Message &content, Packet &packet) {
        packet.set_packetid(static_cast<int>(messagetype));
        google::protobuf::Any *content_ = packet.mutable_content();
        content_->PackFrom(content);
    }
    void processRevcSocket(const int client_fd);
    int Signup(std::string phone_number, const std::string &password);
    ~Server() { this->Finalize(); }
};

} // namespace ntc
