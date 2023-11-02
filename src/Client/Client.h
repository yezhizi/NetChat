#pragma once
#include<sys/socket.h>
#include "client_van.h"
#include "logging.h"
#include <mutex>
class Client
{
private:
    int _client_id;
    Van* _van;

    void Init();
    void Finalize();

    static std::shared_ptr<Client> _getsharedPtr() {
        static std::shared_ptr<Client> instance_ptr(new Client());
        return instance_ptr;
    }

    Client() { this->Init(); }
    Client(const Client&) = delete;
    Client& operator=(const Client&) = delete;


public:
    static inline Client* getInstance() {
        return _getsharedPtr().get();
    }
    static inline std::shared_ptr<Client> getSharedPtr() {
        return _getsharedPtr();
    }
    int Signup( std::string phone_number, const std::string &password);
    ~Client() { this->Finalize(); }

};
