#pragma once
#include<sys/socket.h>
#include "server_van.h"
#include "logging.h"
#include <mutex>
#include "user_manager.h"
namespace ntc{

class Server
{
private:
    Van* _van;
    UM* _um;

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
    const Van * getVan() const { return this->_van; }
    const UM * getUM() const { return this->_um; }

    int Signup( std::string phone_number, const std::string &password);
    ~Server() { this->Finalize(); }

};


}

