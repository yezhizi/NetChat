#ifndef _USER_MANNAER_H
#define _USER_MANNAER_H
// TODO UM类，提供数据库接口，存放用户信息（账号、密码、昵称）、缓存消息
// 连接管理，用户与socket的映射,直接存内存
#include "Server.h"
#include <memory>
#include <string>
#include <unordered_map>
namespace ntc {

class UM {
    friend class Server;

  private:
    static inline std::shared_ptr<UM> getSharedPtr() {
        static std::shared_ptr<UM> instance_ptr(new UM());
        return instance_ptr;
    }

    inline void BindServer(Server *server_ptr) {
        if (server_ptr_ == nullptr)
            server_ptr_ = server_ptr;
    }
    /////////////////////////

    UM() : server_ptr_(nullptr){};
    Server *server_ptr_; // constant pointer
    UM(const UM &) = delete;
    UM &operator=(const UM &) = delete;

    // mutex
    std::mutex _temp_socket_pool_mu;

  public:
    static inline UM *Get() { return getSharedPtr().get(); }
    // TODO 在表中找到对应的长连接socket
    int getLongLifeSocket(const std::string &user_id) { return 0; }
    // TODO 在表中设置对应的长连接socket
    int setLongLifeSocket(const std::string &user_id, int fd) { return 0; }
    // 临时连接池
    // 设置对应的临时连接socket  ipport -> fd threadsafe
    void setTempSocket(const std::string &ipport, int fd) {
        std::lock_guard<std::mutex> lock(this->_temp_socket_pool_mu);
        this->server_ptr_->_temp_socket_pool[ipport] = fd;
    }
};
} // namespace ntc
#endif //_USER_MANNAER_H
