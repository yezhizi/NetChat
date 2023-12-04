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
void Server::processRevcSocket(const int client_fd) {
    // TODO
    Packet msg;
    this->_van->Recv(client_fd, &msg);
    int packet_id = msg.packetid();
    MessageType type = static_cast<MessageType>(packet_id);
    Packet packet_back;
    switch (type) {
    case MessageType::ServerStatusRequest: {
        //ServerStatusRequest
        ServerStatusResponse response;
        response.set_online(true);
        response.set_registrable(false);

        this->packtoPacket(MessageType::ServerStatusResponse,response,packet_back);
    }
    case MessageType::ServerStatusUpdateRequest:{
        //ServerStatusUpdateResponse
        ServerStatusUpdateResponse response;
        response.set_online(true);
        response.set_registrable(false);
        
        this->packtoPacket(MessageType::ServerStatusUpdateResponse,response,packet_back);
    }
    //TODO
    }
    this->_van->Send(packet_back, client_fd);

}
} // namespace ntc