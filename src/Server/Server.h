#pragma once
#include "logging.h"
#include "van.h"

#include <mutex>
#include <sys/socket.h>
#include <unordered_map>
namespace ntc {

class Server {
  friend class UM;
  private:
    Van *_van;

    void Init();
    void Finalize();

    Server() { this->Init(); }
    Server(const Server &) = delete;
    Server &operator=(const Server &) = delete;

    //临时连接池
        std::unordered_map<std::string, int> _temp_socket_pool;


  public:
    static Server *Get() {
        static Server server;
        return &server;
    }
    const Van *getVan() const { return this->_van; }

    int Signup(std::string phone_number, const std::string &password);
    ~Server() { this->Finalize(); }
};

} // namespace ntc
