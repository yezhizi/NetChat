#pragma once
#include<sys/socket.h>
#include "server_van.h"
#include "logging.h"
#include <mutex>
class Server
{
private:
    Van* _van;

    void Init();
    void Finalize();

    static std::shared_ptr<Server> _getsharedPtr() {
        static std::shared_ptr<Server> instance_ptr(new Server());
        return instance_ptr;
    }

    Server() { this->Init(); }
    Server(const Server&) = delete;
    Server& operator=(const Server&) = delete;


public:
    static inline Server* getInstance() {
        return _getsharedPtr().get();
    }
    static inline std::shared_ptr<Server> getSharedPtr() {
        return _getsharedPtr();
    }
    int Signup( std::string phone_number, const std::string &password);
    ~Server() { this->Finalize(); }

};




