#ifndef _SERVER_VAN_H
#define _SERVER_VAN_H

#include "van.h"
#include "base.h"
#include "logging.h"
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <mutex>
#include <unordered_map>
#include <thread>
#include <sys/epoll.h>
namespace ntc{

const int MAX_EVENTS = 10;

class ServerVan : public Van
{
public:
    ServerVan() : Van() {
        
        this->_socket = socket(AF_INET, SOCK_STREAM, 0);
        LOG(INFO) << "ServerVan initialized";
        this->Bind();
        this->epoll_fd_ = epoll_create(MAX_EVENTS);
        struct epoll_event event;
        event.events = EPOLLIN;
        event.data.fd = this->_socket;

        epoll_ctl(this->epoll_fd_, EPOLL_CTL_ADD, this->_socket, &event);

        this->connecting_thread_ = std::unique_ptr<std::thread>(new std::thread(&ServerVan::Connect, this));
    
    

    }
    ~ServerVan() {
        this->connecting_thread_->join();

    }
protected:
    void Bind() override{
        LOG_IF(this->_socket < 0, FATAL) << "create server socket failed";
        struct sockaddr_in server_addr;
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(ntc::kServerPort);
        server_addr.sin_addr.s_addr = inet_addr(ntc::kServerIP.c_str());
        int ret = bind(this->_socket, (struct sockaddr*)&server_addr, sizeof(server_addr));
        LOG_IF(ret < 0, FATAL) << "bind server socket failed";
        LOG(INFO) << "bind server socket success";
        ret = listen(this->_socket, 10);
        LOG_IF(ret < 0, FATAL) << "listen server socket failed";
        LOG(INFO) << "listen server socket success";
    }
    void Connect() override{
        //TODO : modify the epoll logic
        while(true){
            struct epoll_event events[MAX_EVENTS];
            int num_events = epoll_wait(this->epoll_fd_, events, MAX_EVENTS, -1);
            
            for(int i=0;i<num_events;++i){
                if (events[i].data.fd==this->_socket){
                    //process new connection
                    struct sockaddr_in client_addr;
                    int client_addr_len = sizeof(client_addr);
                    int client_socket = accept(this->_socket, (struct sockaddr*)&client_addr, (socklen_t*)&client_addr_len);
                    if(client_socket < 0){
                        LOG(WARNING) << "accept client socket failed";
                        continue;
                    }else{
                        char client_ip[INET_ADDRSTRLEN];
                        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
                        int client_port = ntohs(client_addr.sin_port);
                        std::string client_ip_port = std::string(client_ip) + ":" + std::to_string(client_port);
                        LOG(INFO) << "accept client socket success from "<<client_ip_port;

                        {
                            std::lock_guard<std::mutex> lk(this->_mtx);
                            this->_connected_sockets_map[client_ip_port] = client_socket;
                        }
                        
                        struct epoll_event event;   
                        event.events = EPOLLIN | EPOLLET;
                        event.data.fd = client_socket;
                        epoll_ctl(this->epoll_fd_, EPOLL_CTL_ADD, client_socket, &event);
                    }

                }
            }
        }

    }
    int SendMesg(const Packet& msg,const std::string to ,const std::string from) override{
        //TODO 
        return 0;
    }
    int RecvMesg(Packet* msg) override{
        //TODO
        return 0;
    }

    

private:
    
    std::unique_ptr<std::thread> connecting_thread_;

    int epoll_fd_;
    std::mutex _mtx;
    std::unordered_map<std::string,int> _connected_sockets_map;
};


    

}


#endif //_CLIENT_VAN_H