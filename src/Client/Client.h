#pragma once
#include<shared_ptr>
#include<sys/socket.h>

namespace SERV{
    const std::string ip = "127.0.0.1";
    const int port = 8888;
}


class Client
{
private:
    int _client_id;
    int _client_socket;
    int _client_port;
    
    struct sockaddr_in _server;
    

    static Client* _instance;

    void Init();


    Client();
    ~Client();

public:
    static inline Client* getInstance(){
        if(_instance == nullptr){
            _instance = new Client();
        }
        return _instance;
    }
    void Finalize(){
        if(_instance != nullptr){
            delete _instance;
            _instance = nullptr;
        }
    }

    
};
