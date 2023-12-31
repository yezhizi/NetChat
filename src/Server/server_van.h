#ifndef _SERVER_VAN_H
#define _SERVER_VAN_H

#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/tcp.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <mutex>
#include <thread>
#include <unordered_map>

#include "Server.h"
#include "threadsafe_queue.h"
#include "user_manager.h"
namespace ntc {
class Server;
const int MAX_EVENTS = 10;

class ServerVan : public Van {
  using Packet = netdesign2::Packet;

 public:
  ServerVan() : Van() {
    CLOG(INFO, "Van") << "ServerVan initialized";
    this->Bind();

    this->RecvSocketThreads_.resize(Server::getRecvSocketNum());
    for (auto &t : this->RecvSocketThreads_)
      t = std::thread(&ServerVan::RevcSocketThreadFunc_, this);

    int epoll_fd_ = epoll_create(MAX_EVENTS);
    CLOG_IF(epoll_fd_ < 0, FATAL, "Van") << "epoll_create failed";
    this->epoll_fd_ = epoll_fd_;

    struct epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = this->_socket;

    int ret = epoll_ctl(this->epoll_fd_, EPOLL_CTL_ADD, this->_socket, &event);
    CLOG_IF(ret < 0, FATAL, "Van") << "epoll_ctl add server socket failed";
    this->accepting_thread_ = std::make_unique<std::thread>(
        &ServerVan::Accepting, this);
    LOG(INFO) << "ServerVan accepting thread started";
  }

  int Control(const int dst, const std::string &cmd = "") override {
    if (cmd == "EPOLL_DEL_FD") {
      return this->controlEpollDel(dst);
    } else if (cmd == "CLOSE_FD") {
      LOG(INFO) << "close client socket " << dst;
      return close(dst);
    } else {
      return -1;
    }
  }

  // epoll 取消监听事件
  int controlEpollDel(int fd) {
    CLOG(INFO, "Van") << "epoll_ctl del client socket " << fd;
    struct epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = fd;

    int ret = epoll_ctl(this->epoll_fd_, EPOLL_CTL_DEL, fd, &event);
    CLOG_IF(ret < 0, FATAL, "Van") << "epoll_ctl del client socket failed";
    return ret;
  }
  ~ServerVan() {
    this->accepting_thread_->join();
    for (auto &t : this->RecvSocketThreads_) t.join();
    for (auto &t : this->KeepAliveThreads_) t.join();
    // TODO : need to do
  }
  void addSendTask(int fd, const Packet &msg) override {
    this->KeepAliveQueue_.Push(std::make_pair(fd, msg));
  }

 protected:
  void Bind() override {
    CLOG_IF(this->_socket < 0, FATAL, "Van") << "create server socket failed";
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(ntc::kServerPort);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    int opt = 1;
    setsockopt(this->_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    int ret = bind(this->_socket, (struct sockaddr *)&server_addr,
                   sizeof(server_addr));
    CLOG_IF(ret < 0, FATAL, "Van") << "bind server socket failed";
    CLOG(INFO, "Van") << "bind server socket success";
    ret = listen(this->_socket, 10);
    CLOG_IF(ret < 0, FATAL, "Van") << "listen server socket failed";
    CLOG(INFO, "Van") << "listen server socket success";
  }

  void Accepting() override {
    while (true) {
      struct epoll_event events[MAX_EVENTS];
      int num_events = epoll_wait(this->epoll_fd_, events, MAX_EVENTS, -1);
      LOG(DEBUG) << "epoll_wait return " << num_events;
      for (int i = 0; i < num_events; ++i) {
        if (events[i].data.fd == this->_socket) {
          // process new connection
          struct sockaddr_in client_addr;
          int client_addr_len = sizeof(client_addr);
          int client_socket =
              accept(this->_socket, (struct sockaddr *)&client_addr,
                     (socklen_t *)&client_addr_len);
          if (client_socket < 0) {
            CLOG(WARNING, "Van") << "accept client socket failed";
            continue;
          }
          char client_ip[INET_ADDRSTRLEN];
          inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
          int client_port = ntohs(client_addr.sin_port);
          std::string client_ip_port =
              std::string(client_ip) + ":" + std::to_string(client_port);
          CLOG(INFO, "Van")
              << "accept client socket success from " << client_ip_port;

          UM::Get()->setRevcSocketMp(client_ip_port, client_socket);

          // 设置nodelay
          int opt = 1;
          setsockopt(client_socket, IPPROTO_TCP, TCP_NODELAY, &opt,
                     sizeof(opt));

          // 设置非阻塞接收
          int flags = fcntl(client_socket, F_GETFL, 0);
          int ret = fcntl(client_socket, F_SETFL, flags | O_NONBLOCK);
          CLOG_IF(ret < 0, WARNING, "Van") << "fcntl set nonblock failed";

          struct epoll_event event;
          event.events = EPOLLIN | EPOLLET | EPOLLHUP | EPOLLERR;
          event.data.fd = client_socket;
          ret =
              epoll_ctl(this->epoll_fd_, EPOLL_CTL_ADD, client_socket, &event);
          CLOG_IF(ret < 0, FATAL, "Van")
              << "epoll_ctl add client socket failed";
        } else {
          // the client socket is ready to read, add to the queue
          if (events[i].events & EPOLLIN) {
            CLOG(DEBUG, "Van")
                << "client socket ready to read :" << events[i].data.fd;
            this->RecvSocketQueue_.Push(events[i].data.fd);
          } else {
            // 连接断开事件
            CLOG(INFO, "Van") << "client socket disconnected";
            this->Control(events[i].data.fd, "EPOLL_DEL_FD");
            this->Control(events[i].data.fd, "CLOSE_FD");
          }
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
      int size_send = send(fd, buf + bytes, len - bytes, MSG_NOSIGNAL);
      if (size_send < 0) {
        if (errno == EPIPE) {
          // 远程主机关闭
          // 让server van epoll_ctl删除对应的监听事件,并关闭socket
          // this->Control(fd, "EPOLL_DEL_FD");
          // this->Control(fd, "CLOSE_FD");
        }
        CLOG(WARNING, "Van") << "send message failed";
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
          // 没有数据可读
          break;
        } else {
          CLOG(WARNING, "Van") << "recv failed";
          return -1;
        }
      } else if (ret == 0) {
        // 远程主机关闭
        break;

      } else {
        bytes += ret;
        if (bytes >= ntc::kMaxMessageSize) {
          CLOG(WARNING, "Van") << "message too large";
          return -1;
        }
      }
    }
    bool ret = msg->ParseFromArray(buf, bytes);
    if (!ret) return -1;
    CLOG(DEBUG, "Van") << "recieve bytes: " << bytes;
    return bytes;
  }

 private:
  std::unique_ptr<std::thread> accepting_thread_;
  // 临时连接池
  ThreadsafeQueue<int> RecvSocketQueue_;
  // 工作在临时连接池的工作线程
  std::vector<std::thread> RecvSocketThreads_;

  // 保活连接池 待发送的消息队列
  ThreadsafeQueue<std::pair<int, Packet>> KeepAliveQueue_;
  std::vector<std::thread> KeepAliveSendThreads_;

  // 工作在保活连接池的工作线程
  std::vector<std::thread> KeepAliveThreads_;
  int epoll_fd_;
  std::mutex _mtx;

  // 工作线程函数
  void RevcSocketThreadFunc_() {
    while (true) {
      int client_fd;
      this->RecvSocketQueue_.WaitAndPop(&client_fd);
      Server::Get().processRecvSocket(client_fd);
    }
  }
};

}  // namespace ntc

#endif  //_CLIENT_VAN_H