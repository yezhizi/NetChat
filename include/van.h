#ifndef _VAN_H
#define _VAN_H
#include <atomic>
#include <mutex>
#include <thread>
#include <string>
#include "message.h"
#include "meta.pb.h"
class PBMeta;
class Van {
public:
    int Send(const Message& msg);
    Van();
    virtual ~Van() {}
protected:
    virtual void Connect() = 0;
    virtual void Bind() = 0;
    virtual int SendMesg(const Message& msg) = 0;
    virtual int RecvMesg(Message* msg) = 0;
    void packMeta(const Meta& meta, char** buf, int* size);

private:
    int _socket;
    void Receiving();
    std::unique_ptr<std::thread> receiving_thread_;
    std::atomic<size_t> send_bytes_{0};
};





#endif //_VAN_H