#ifndef _USER_MANNAER_H
#define _USER_MANNAER_H
// TODO UM类，提供数据库接口，存放用户信息（账号、密码、昵称）、缓存消息
// 连接管理，用户与socket的映射,直接存内存
#include <memory>
#include <string>
#include <unordered_map>

#include "db_access.h"
#include "Server.h"

namespace ntc {

class UM {
  friend class Server;

 private:
  Server *server_ptr_;  // constant pointer

  // mutex
  std::mutex _temp_socket_pool_mu;
  std::mutex _challenge_pool_mu;
  std::mutex __keepalive_socket_pool_mu;

  UM() : server_ptr_(nullptr){};
  UM(const UM &) = delete;
  UM &operator=(const UM &) = delete;

  static inline std::shared_ptr<UM> getSharedPtr() {
    static std::shared_ptr<UM> instance_ptr(new UM());
    return instance_ptr;
  }

  inline void BindServer(Server *server_ptr) {
    if (server_ptr_ == nullptr) server_ptr_ = server_ptr;
  }

 public:
  static UM *Get() { return getSharedPtr().get(); }

  // TODO 析构函数

  // TODO 在表中找到对应的长连接socket
  int getLongLifeSocket(const std::string &user_id) { return 0; }
  // TODO 在表中设置对应的长连接socket
  int setLongLifeSocket(const std::string &user_id, int fd) { return 0; }
  // 临时连接池
  // 设置对应的临时连接socket  ipport -> fd threadsafe
  void setRevcSocketMp(const std::string &ipport, int fd) {
    std::lock_guard<std::mutex> lock(this->_temp_socket_pool_mu);
    this->server_ptr_->_revc_socket_pool[ipport] = fd;
  }

  // 用户数据 id->int32  username->str  pass->str(sha256 hash)
  // 在线状态 id->int32 token -> str ("" for offline)
  // token->username
  std::string getUsernameByToken(const std::string &token) {
    if (token == "") return "";
    std::lock_guard<std::mutex> lock(this->__keepalive_socket_pool_mu);
    if (this->server_ptr_->_token_pool.find(token) !=
        this->server_ptr_->_token_pool.end()) {
      return this->server_ptr_->_token_pool[token];
    } else {
      return "";
    }
  }
  // 消息

  /////////////////////////////////////////////////////////////////////////
  ///// 用户登录 ////
  //  设置对应的challenge  username -> challenge threadsafe
  // 返回用户是否注册
  bool setChallengeMp(const std::string &username,
                      const std::string &challenge) {
    // 查找对应的用户是否注册
    auto result = g_db->getUser(username);
    if (!result.has_value()) {
      return false;
    }

    std::lock_guard<std::mutex> lock(this->_challenge_pool_mu);
    this->server_ptr_->_challenge_pool[username] = challenge;
    return true;
  }

  // 获取对应的challenge  username -> challenge threadsafe
  std::string getChallengeMp(const std::string &username) {
    std::lock_guard<std::mutex> lock(this->_challenge_pool_mu);
    if (this->server_ptr_->_challenge_pool.find(username) !=
        this->server_ptr_->_challenge_pool.end()) {
      return this->server_ptr_->_challenge_pool[username];
    } else {
      return "";
    }
  }

  // 删除对应的challenge  username -> challenge threadsafe
  void delChallengeMp(const std::string &username) {
    std::lock_guard<std::mutex> lock(this->_challenge_pool_mu);
    if (this->server_ptr_->_challenge_pool.find(username) !=
        this->server_ptr_->_challenge_pool.end()) {
      this->server_ptr_->_challenge_pool.erase(username);
    }
  }

  // 保活连接池
  //  设置对应的保活连接socket
  void setKeepaliveSocketMp(const std::string &username, int fd,
                            const std::string &token) {
    std::lock_guard<std::mutex> lock(this->__keepalive_socket_pool_mu);
    this->server_ptr_->_keepalive_socket_pool[username] = fd;
    this->server_ptr_->_token_pool[token] = username;
  }

  // username->fd
  int getfdByUsername(const std::string &username) {
    if (username == "") return -1;
    std::lock_guard<std::mutex> lock(this->__keepalive_socket_pool_mu);
    if (this->server_ptr_->_keepalive_socket_pool.find(username) !=
        this->server_ptr_->_keepalive_socket_pool.end()) {
      return this->server_ptr_->_keepalive_socket_pool[username];
    } else {
      return -1;
    }
  }
};
}  // namespace ntc
#endif  //_USER_MANNAER_H
