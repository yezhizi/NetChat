#include "Client.h"
#include "client_van.h"
#include "crypto_utils.h"
namespace ntc {
void Client::Init() {
    this->_van = (Van *)new ClientVan();
}
void Client::Finalize() {
    delete this->_van;
    CLOG(INFO, "client") << "Client closed";
}
void Client::sendServerStatusRequest() {
    Packet packet;
    ServerStatusRequest serverStatusRequest;
    this->packtoPacket(MessageType::ServerStatusRequest, serverStatusRequest,
                       packet);

    CLOG(INFO, "client") << "Client sent ServerStatusRequest";
    this->_van->Send(packet, this->_van->test_getSocket());

    // revc
    packet.Clear();
    this->_van->Recv(this->_van->test_getSocket(), &packet);
    CLOG(INFO, "client") << "Client received packetid: " << packet.packetid();
    ServerStatusResponse serverStatusResponse;
    packet.content().UnpackTo(&serverStatusResponse);
    CLOG(INFO, "client") << "Client received ServerStatusResponse";
    CLOG(INFO, "client") << "ServerStatusResponse: "
                         << "online: " << serverStatusResponse.online() << " "
                         << "registrable" << serverStatusResponse.registrable()
                         << " "
                         << "serverPublicKey"
                         << serverStatusResponse.serverpublickey();
}


void Client::sendServerStatusUpdateRequest() {
    Packet packet;
    ServerStatusUpdateRequest serverStatusUpdateRequest;
    this->packtoPacket(MessageType::ServerStatusUpdateRequest,
                       serverStatusUpdateRequest, packet);

    CLOG(INFO, "client") << "Client sent ServerStatusUpdateRequest";
    this->_van->Send(packet, this->_van->test_getSocket());

    // revc
    packet.Clear();
    this->_van->Recv(this->_van->test_getSocket(), &packet);
    CLOG(INFO, "client") << "Client received packetid: " << packet.packetid();
    ServerStatusUpdateResponse serverStatusUpdateResponse;
    packet.content().UnpackTo(&serverStatusUpdateResponse);
    CLOG(INFO, "client") << "Client received ServerStatusUpdateResponse";
    CLOG(INFO, "client") << "ServerStatusUpdateResponse: "
                         << "online: " << serverStatusUpdateResponse.online()
                         << " "
                         << "registrable: "
                         << serverStatusUpdateResponse.registrable();
}

void Client::login(const std::string &username, const std::string &password) {
    Packet packet;
    LoginPreRequest loginPreRequest;
    loginPreRequest.set_username(username);
    this->packtoPacket(MessageType::LoginPreRequest, loginPreRequest, packet);
    this->_van->Send(packet, this->_van->test_getSocket());
    CLOG(DEBUG, "client") << "Client sent LoginPreRequest "
                          << "username: " << username;
    // recv loginPreResponse
    packet.Clear();
    this->_van->Recv(this->_van->test_getSocket(), &packet);
    LoginPreResponse loginPreResponse;
    packet.content().UnpackTo(&loginPreResponse);
    std::string challenge = loginPreResponse.challenge();
    CLOG(INFO, "client") << "Client received packetid: " << packet.packetid();
    // 计算(pass.sha256+challenge).sha256
    std::string hashPassword = SHA256(password);
    std::string passChallenge = SHA256(hashPassword + challenge);
    // send loginRequest
    packet.Clear();
    LoginRequest loginRequest;
    loginRequest.set_username(username);
    loginRequest.set_hashpassword(passChallenge);
    this->packtoPacket(MessageType::LoginRequest, loginRequest, packet);
    this->_van->Send(packet, this->_van->test_getSocket());
    CLOG(DEBUG, "client") << "Client sent LoginRequest "
                          << "username: " << username ;
    //recv loginResponse
    packet.Clear();
    this->_van->Recv(this->_van->test_getSocket(), &packet);
    LoginResponse loginResponse;
    packet.content().UnpackTo(&loginResponse);
    CLOG(INFO, "client") << "Client received packetid: " << packet.packetid();
    CLOG(DEBUG, "client") << "loginResponse: "
                          << "login: " << loginResponse.logined() << " "
                          << "token: " << loginResponse.token();
    this->_token = loginResponse.token();
}

    void Client::setupChannel(){
        if (this->_token.empty()) {
            CLOG(ERROR, "client") << "token is empty";
            return;
        }
        CLOG(DEBUG, "client") << "Client sent SetupChannelRequest";
        Packet packet;
        SetupChannelRequest setupChannelRequest;
        setupChannelRequest.set_token(this->_token);
        this->packtoPacket(MessageType::SetupChannelRequest, setupChannelRequest,
                           packet);
        this->_van->Send(packet, this->_van->test_getSocket());
        // recv SetupChannelResponse
        packet.Clear();
        this->_van->Recv(this->_van->test_getSocket(), &packet);
        ServerAckResponse serverAckResponse;
        packet.content().UnpackTo(&serverAckResponse);
        CLOG(INFO, "client") << "Client received packetid: " << packet.packetid();
        CLOG(DEBUG, "client") << "ServerAckResponse: ";

    }

} // namespace ntc