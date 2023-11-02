#ifndef _CLIENT_VAN_H
#define _CLIENT_VAN_H

#include "van.h"
#include "base.h"
#include "logging.h"
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include "message.h"
#include <mutex>
class ClientVan : public Van
{
public:
    ClientVan():Van(){
        LOG(INFO) << "ClientVan initialized";
        this->Connect();
    }
    ~ClientVan(){
        
    }
    enum class ClientStatus {OFFLINE , ONLINE};
protected:
    void Connect() override{
        if(this->_status == ClientStatus::ONLINE){
            return;
        }
        struct sockaddr_in server_addr;
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(ntc::kServerPort);
        server_addr.sin_addr.s_addr = inet_addr(ntc::kServerIP.c_str());
        int ret = connect(this->_socket, (struct sockaddr*)&server_addr, sizeof(server_addr));
        if(ret < 0){
            LOG(ERROR) << "connect to server "<<ntc::kServerIP<<":"<<ntc::kServerPort<<" failed";
        }
        else{
            LOG(INFO ) << "connect to server "<<ntc::kServerIP<<":"<<ntc::kServerPort<<" success";
            this->_status = ClientStatus::ONLINE;
        }
        
    }
    int SendMesg(const Message& msg) override
    {
        std::lock_guard<std::mutex> lk(this->_mtx);
        if (this->_socket < 0 || this->_status == ClientStatus::OFFLINE)
        {
            LOG(ERROR) << "socket is not connected";
            return -1;
        }
        // send the meta
        int meta_size; char* meta_buf;
        packMeta(msg.meta, &meta_buf, &meta_size);
        
        while(true){
            if(send(this->_socket, meta_buf, meta_size, 0) == meta_size) break;
            if (errno == EINTR) continue;
            LOG(WARNING) << "fail send meta to server, error no is "<< errno;
            return -1;
        }
        int send_bytes = meta_size;
        for (auto r : msg.data)
        {
            while(true){
                if(send(this->_socket, r.data(), r.size(), 0) == r.size()) break;
                if (errno == EINTR) continue;
                LOG(WARNING) << "fail send data to server, error no is "<< errno;
                return -1;
            }
            send_bytes += r.size();
        }

        return send_bytes;
    }
    int RecvMesg(Message* msg) override
    {
        return 0;
    }
    void Bind() override{}

private:
    ClientStatus _status;
    int _socket;
    std::mutex _mtx;
};



#endif //_CLIENT_VAN_H