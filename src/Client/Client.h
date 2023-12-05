#pragma once
#include "client_van.h"
#include "logging.h"
#include "message.h"
#include <mutex>
#include <sys/socket.h>
namespace ntc {
class Client {
  private:
    Van *_van;

    void Init();
    void Finalize();

    static void packtoPacket(MessageType messagetype,
                             google::protobuf::Message &content,
                             Packet &packet) {
        packet.set_packetid(static_cast<int>(messagetype));
        google::protobuf::Any *content_ = packet.mutable_content();
        content_->PackFrom(content);
    }

  public:
    Client() { this->Init(); }
    ~Client() { this->Finalize(); }

    void sendServerStatusRequest();
    void sendServerStatusUpdateRequest();
    void login(const std::string &username, const std::string &password);
};
} // namespace ntc