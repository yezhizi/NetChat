#ifndef _SERVER_VAN_H
#define _SERVER_VAN_H

#include "Server.h"
#include "base.h"
#include "logging.h"
#include "threadsafe_queue.h"
#include "user_manager.h"
#include "van.h"
#include <arpa/inet.h>
#include <mutex>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>
#include <unordered_map>

#include <fcntl.h>
namespace ntc {
class Server;
const int MAX_EVENTS = 10;

class ServerVan : public Van {
  public:
    ServerVan() : Van() {
        this->_socket = socket(AF_INET, SOCK_STREAM, 0);
        LOG(INFO) << "ServerVan initialized";
        this->Bind();

        this->RevcSocketThreads_.resize(Server::getRevcSocketNum());
        for (auto &t : this->RevcSocketThreads_)
            t = std::thread(&ServerVan::RevcSocketThreadFunc_, this);
        // bind the working thread function

        int epoll_fd_ = epoll_create(MAX_EVENTS);
        LOG_IF(epoll_fd_ < 0, FATAL) << "epoll_create failed";
        this->epoll_fd_ = epoll_fd_;

        struct epoll_event event;
        event.events = EPOLLIN;
        event.data.fd = this->_socket;

        int ret =
            epoll_ctl(this->epoll_fd_, EPOLL_CTL_ADD, this->_socket, &event);
        LOG_IF(ret < 0, FATAL) << "epoll_ctl add server socket failed";
        this->accepting_thread_ = std::unique_ptr<std::thread>(
            new std::thread(&ServerVan::Accepting, this));
    }
    ~ServerVan() { this->accepting_thread_->join(); }

  protected:
    void Bind() override {
        LOG_IF(this->_socket < 0, FATAL) << "create server socket failed";
        struct sockaddr_in server_addr;
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(ntc::kServerPort);
        server_addr.sin_addr.s_addr = inet_addr(ntc::kServerIP.c_str());
        int ret = bind(this->_socket, (struct sockaddr *)&server_addr,
                       sizeof(server_addr));
        LOG_IF(ret < 0, FATAL) << "bind server socket failed";
        LOG(INFO) << "bind server socket success";
        ret = listen(this->_socket, 10);
        LOG_IF(ret < 0, FATAL) << "listen server socket failed";
        LOG(INFO) << "listen server socket success";
    }
    void Accepting() override {
        // TODO : modify the epoll logic
        while (true) {
            struct epoll_event events[MAX_EVENTS];
            int num_events =
                epoll_wait(this->epoll_fd_, events, MAX_EVENTS, -1);

            for (int i = 0; i < num_events; ++i) {
                if (events[i].data.fd == this->_socket) {
                    // process new connection
                    struct sockaddr_in client_addr;
                    int client_addr_len = sizeof(client_addr);
                    int client_socket =
                        accept(this->_socket, (struct sockaddr *)&client_addr,
                               (socklen_t *)&client_addr_len);
                    if (client_socket < 0) {
                        LOG(WARNING) << "accept client socket failed";
                        continue;
                    }
                    char client_ip[INET_ADDRSTRLEN];
                    inet_ntop(AF_INET, &client_addr.sin_addr, client_ip,
                              INET_ADDRSTRLEN);
                    int client_port = ntohs(client_addr.sin_port);
                    std::string client_ip_port = std::string(client_ip) + ":" +
                                                 std::to_string(client_port);
                    LOG(INFO) << "accept client socket success from "
                              << client_ip_port;

                    UM::Get()->setRevcSocketMp(client_ip_port, client_socket);

                    // 设置非阻塞接收
                    int flags = fcntl(client_socket, F_GETFL, 0);
                    int ret = fcntl(client_socket, F_SETFL, flags | O_NONBLOCK);
                    LOG_IF(ret < 0, WARNING) << "fcntl set nonblock failed";

                    struct epoll_event event;
                    event.events = EPOLLIN | EPOLLET;
                    event.data.fd = client_socket;
                    ret = epoll_ctl(this->epoll_fd_, EPOLL_CTL_ADD,
                                    client_socket, &event);
                    LOG_IF(ret < 0, FATAL)
                        << "epoll_ctl add client socket failed";
                } else {
                    // the client socket is ready to read, add to the queue
                    if (events[i].events & EPOLLIN)
                        this->RevcSocketQueue_.Push(events[i].data.fd);
                }
            }
        }
    }
    int SendMesg(const Packet &msg, const std::string to,
                 const std::string from) override {
        // TODO
        return 0;
    }
    int SendMesg(const Packet &msg, const int fd) override {
        int bytes = 0;
        char buf[ntc::kMaxMessageSize];
        msg.SerializeToArray(buf, ntc::kMaxMessageSize);
        int len = msg.ByteSizeLong();
        while (bytes < len) {
            int size_send = send(fd, buf + bytes, len - bytes, 0);
            if (size_send < 0) {
                LOG(ERROR) << "send message failed";
                return -1;
            } else {
                bytes += size_send;
            }
        }
        return bytes;
    }
    int RecvMesg(int fd, Packet *msg) override {
        // TODO
        int bytes = 0;
        char buf[ntc::kMaxMessageSize];

        while (true) {
            int ret = recv(fd, buf + bytes, ntc::kMaxMessageSize - bytes, 0);
            if (ret < 0) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    break;
                } else {
                    LOG(WARNING) << "recv failed";
                    return -1;
                }
            } else if (ret == 0) {
                LOG(WARNING) << "client closed";
                return 0;
            } else {
                bytes += ret;
                if (bytes >= ntc::kMaxMessageSize) {
                    LOG(WARNING) << "message too large";
                    return -1;
                }
            }
        }
        msg->ParseFromArray(buf, bytes);
        return bytes;
    }

  private:
    std::unique_ptr<std::thread> accepting_thread_;
    // 临时连接池
    ThreadsafeQueue<int> RevcSocketQueue_;
    // 工作在临时连接池的工作线程
    std::vector<std::thread> RevcSocketThreads_;

    // 保活连接池 待发送的消息队列
    ThreadsafeQueue<std::pair<int, Packet>> KeepAliveQueue_;
    // 工作在保活连接池的工作线程
    std::vector<std::thread> KeepAliveThreads_;
    int epoll_fd_;
    std::mutex _mtx;

    // 工作线程函数
    void RevcSocketThreadFunc_() {
        while (true) {
            int client_fd;
            this->RevcSocketQueue_.WaitAndPop(&client_fd);
            Server::Get()->processRevcSocket(client_fd);
        }
    }
};

} // namespace ntc

#endif //_CLIENT_VAN_H