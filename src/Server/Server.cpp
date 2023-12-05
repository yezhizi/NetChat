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
    Packet packet_back;
    switch (type) {
    case MessageType::ServerStatusRequest: {
        // ServerStatusRequest
        ServerStatusResponse response;
        response.set_online(true);
        response.set_registrable(false);

        Server::packtoPacket(MessageType::ServerStatusResponse, response,
                             packet_back);
        LOG(INFO) << "Server sent ServerStatusResponse"
                  << " online: " << response.online()
                  << " registrable: " << response.registrable();
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
        LOG(INFO) << "Server sent ServerStatusUpdateResponse"
                  << " online: " << response.online()
                  << " registrable: " << response.registrable();
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
        char *challenge = new char[ntc::kChallengeSize];
        ntc::genChallenge(challenge);
        bool ret = UM::Get()->setChallengeMp(username, challenge);
        if (!ret) {
            // 发现用户未注册
            LOG(INFO) << "User not registered. username: " << username;
            response.set_challenge("");
        } else {
            response.set_challenge(challenge);
        }
        delete[] challenge;
        this->packtoPacket(MessageType::LoginPreResponse, response,
                           packet_back);
        LOG(INFO) << "Server sent LoginPreResponse";
        break;
    }
    case MessageType::LoginRequest: {
        // LoginResponse
        LoginRequest request;
        msg.content().UnpackTo(&request);
        std::string username = request.username();
        std::string hashPassword = request.hashpassword();
        // 获取记录的challenge
        std::string challenge = UM::Get()->getChallengeMp(username);
        // 获取记录的pass.sha256
        std::string pass_sha256 = UM::Get()->getUserInfo(username).second;
        LoginResponse response;
        if (challenge == "" || pass_sha256 == "") {
            // 发现用户未注册
            LOG(INFO) << "User not find. username: " << username;
            response.set_logined(false);
            response.set_token("illegal user");
            this->packtoPacket(MessageType::LoginResponse, response,
                               packet_back);
            break;
        }
        // 计算(pass.sha256+challenge).sha256
        std::string pass_challenge_sha256 =
            ntc::SHA256(pass_sha256 + challenge);
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
        UM::Get()->delChallengeMp(username); // 删除记录的challenge
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
        if (fd==-1 || fd !=client_fd){
            // user not login or token error
            LOG(INFO) << "User not login or token error. username: " << username;
            // TODO 暂时不处理
    
        }
        //  1. 通知van取消对应的监听事件 
        this->_van->Control(client_fd);
        //ServerAckResponse
        ServerAckResponse response;
        LOG(INFO) << "Server sent ServerAckResponse";
        this->packtoPacket(MessageType::ServerAckResponse, response,
                           packet_back);
        // TODO 2. 唤醒在长连接处的工作线程,给客户端发送一些消息(联系人...)
        break;
    }
    }
    int ret = this->_van->Send(packet_back, client_fd);

    if (ret < 0) {
        // 让server van epoll_ctl删除对应的监听事件,并关闭socket
    }
    LOG(INFO) << "Server sent packet. id : " << packet_back.packetid();
}
} // namespace ntc