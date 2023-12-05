#pragma once
#include "client_van.h"
#include "logging.h"
#include "message.h"
#include <mutex>
#include <sys/socket.h>
namespace ntc {
class Client {
  private:
    int _client_id;
    Van *_van;

    void Init();
    void Finalize();

    static std::shared_ptr<Client> _getsharedPtr() {
        static std::shared_ptr<Client> instance_ptr(new Client());
        return instance_ptr;
    }

    Client() { this->Init(); }
    Client(const Client &) = delete;
    Client &operator=(const Client &) = delete;

  public:
    static inline Client *getInstance() { return _getsharedPtr().get(); }
    static inline std::shared_ptr<Client> getSharedPtr() {
        return _getsharedPtr();
    }

    ~Client() { this->Finalize(); }

    void sendServerStatusRequest();
    void sendServerStatusUpdateRequest();
    void login(const std::string &username, const std::string &password);

    static void packtoPacket(MessageType messagetype,
                             google::protobuf::Message &content,
                             Packet &packet) {
        packet.set_packetid(static_cast<int>(messagetype));
        google::protobuf::Any *content_ = packet.mutable_content();
        content_->PackFrom(content);
    }
};
} // namespace ntc