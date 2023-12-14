#ifndef _VAN_H
#define _VAN_H
#include "message.h"
#include <atomic>
#include <mutex>
#include <string>
#include <thread>

namespace ntc {
class Van {
  public:
    int Send(const Packet &msg, const std::string to, const std::string from);
    int Send(const Packet &msg, const int fd);
    int Recv(const int client_fd, Packet *msg);
    virtual int Control(const int dst, const std::string &cmd = "") = 0;
    virtual void addSendTask(const int fd, const Packet &msg) = 0;
    Van();
    virtual ~Van() {}
    int test_getSocket() const { return this->_socket; }

  protected:
    virtual void Accepting() = 0;
    virtual void Bind() = 0;
    virtual int SendMesg(const Packet &msg, const std::string to,
                         const std::string from) = 0;
    virtual int SendMesg(const Packet &msg, const int fd) = 0;
    virtual int RecvMesg(const int fd, Packet *msg) = 0;
    // void packMeta(const Meta& meta, char** buf, int* size);
    int _socket;

  private:
    void Receiving();
    std::unique_ptr<std::thread> receiving_thread_;
    std::atomic<size_t> send_bytes_{0};
    std::atomic<size_t> recv_bytes_{0};
};

} // namespace ntc

#endif //_VAN_H