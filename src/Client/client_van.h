#ifndef _CLIENT_VAN_H
#define _CLIENT_VAN_H

#include "base.h"
#include "logging.h"
#include "message.h"
#include "van.h"
#include <arpa/inet.h>
#include <mutex>
#include <sys/socket.h>
#include <unistd.h>
namespace ntc {
class ClientVan : public Van {
  public:
    ClientVan() : Van() {
        CLOG(INFO, "Van") << "ClientVan initialized";
        this->Accepting();
        // this->receiving_thread_ = std::unique_ptr<std::thread>(
        //     new std::thread(&ClientVan::Receiving, this));
    }
    ~ClientVan() {}
    int Control(const int dst, const std::string &cmd = "") override {
        return 0;
    }
    enum class ClientStatus { OFFLINE, ONLINE };

  protected:
    void Accepting() override {
        if (this->_status == ClientStatus::ONLINE) {
            return;
        }
        struct sockaddr_in server_addr;
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(ntc::kServerPort);
        server_addr.sin_addr.s_addr = inet_addr(ntc::kServerIP.c_str());
        int ret = connect(this->_socket, (struct sockaddr *)&server_addr,
                          sizeof(server_addr));
        if (ret < 0) {
            CLOG(ERROR, "Van") << "connect to server " << ntc::kServerIP << ":"
                               << ntc::kServerPort << " failed";
        } else {
            CLOG(INFO, "Van") << "connect to server " << ntc::kServerIP << ":"
                              << ntc::kServerPort << " success";
            this->_status = ClientStatus::ONLINE;
        }
    }
    int SendMesg(const Packet &msg, const int fd) override {
        int bytes = 0;
        char buf[ntc::kMaxMessageSize];
        msg.SerializeToArray(buf, ntc::kMaxMessageSize);
        int len = msg.ByteSizeLong();
        while (bytes < len) {
            int size_send = send(fd, buf + bytes, len - bytes, 0);
            if (size_send < 0) {
                CLOG(ERROR, "Van") << "send message failed";
                return -1;
            } else {
                bytes += size_send;
            }
        }
        return bytes;
    }
    int RecvMesg(const int fd, Packet *msg) override { // TODO
        char buf[ntc::kMaxMessageSize];

        // 阻塞接收
        int bytes = recv(fd, buf, ntc::kMaxMessageSize, 0);
        if (bytes < 0) {
            CLOG(ERROR, "Van") << "recv message failed";
            return -1;
        }
        msg->ParseFromArray(buf, bytes);
        return bytes;
    }
    int SendMesg(const Packet &msg, const std::string to,
                 const std::string from) override {
        return 0;
    }
    void Bind() override {}

  private:
    std::unique_ptr<std::thread> receiving_thread_;
    ClientStatus _status;
    std::mutex _mtx;
    void Receiving() {}
};

} // namespace ntc
#endif //_CLIENT_VAN_H