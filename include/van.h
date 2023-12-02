#ifndef _VAN_H
#define _VAN_H
#include <atomic>
#include <mutex>
#include <thread>
#include <string>
#include "message.h"

namespace ntc{
class Van {
public:
    int Send(const Packet& msg,const std::string to ,const std::string from);
    Van();
    virtual ~Van() {}
protected:
    virtual void Connect() = 0;
    virtual void Bind() = 0;
    virtual int SendMesg(const Packet& msg,const std::string to ,const std::string from) = 0;
    virtual int RecvMesg(Packet* msg) = 0;
    // void packMeta(const Meta& meta, char** buf, int* size);
    int _socket;
private:

    void Receiving();
    std::unique_ptr<std::thread> receiving_thread_;
    std::atomic<size_t> send_bytes_{ 0 };
};


}



#endif //_VAN_H