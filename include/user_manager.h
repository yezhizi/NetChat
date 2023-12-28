#ifndef _USER_MANNAER_H
#define _USER_MANNAER_H
// TODO UM类，缓存消息
// 连接管理，用户与socket的映射,直接存内存
#include <memory>
#include <string>
#include <unordered_map>

#include "Server.h"

namespace ntc {

class UM {
  friend class Server;
  using KeepAliveMsgSender = Server::KeepAliveMsgSender;

 private:
  Server *server_ptr_;  // constant pointer

  // mutex
  std::mutex _temp_socket_pool_mu;
  std::mutex _challenge_pool_mu;
  std::mutex _keepalive_sender_mp_mu;
  std::mutex _token_pool_mu;
  std::mutex _file_msg_pool_mu;

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

  ///// 用户登录 ////
  // 设置对应的challenge  userid -> challenge threadsafe
  void setChallengeMp(const int &id, const std::string &challenge) {
    std::lock_guard<std::mutex> lock(this->_challenge_pool_mu);
    this->server_ptr_->_challenge_pool[id] = challenge;
  }

  // 获取对应的challenge  id -> challenge threadsafe
  std::string getChallengeMp(const int &id) {
    std::lock_guard<std::mutex> lock(this->_challenge_pool_mu);
    if (this->server_ptr_->_challenge_pool.find(id) !=
        this->server_ptr_->_challenge_pool.end()) {
      return this->server_ptr_->_challenge_pool[id];
    } else {
      return "";
    }
  }

  // 删除对应的challenge  id -> challenge threadsafe
  void delChallengeMp(const int &id) {
    std::lock_guard<std::mutex> lock(this->_challenge_pool_mu);
    if (this->server_ptr_->_challenge_pool.find(id) !=
        this->server_ptr_->_challenge_pool.end()) {
      this->server_ptr_->_challenge_pool.erase(id);
    }
  }

  // 保活连接池
  // 设置对应的保活连接的Sender
  void setKpAliveSender(const int &uid, int fd) {
    std::lock_guard<std::mutex> lock(this->_keepalive_sender_mp_mu);
    LOG(INFO) << "set keepalive sender for user " << uid;
    if (this->server_ptr_->_keepalive_sender_mp.find(uid) !=
        this->server_ptr_->_keepalive_sender_mp.end()) {
      this->server_ptr_->_keepalive_sender_mp[uid]->setFd(fd);
      this->server_ptr_->_keepalive_sender_mp[uid]->Clear();
    } else {
      this->server_ptr_->_keepalive_sender_mp[uid] =
          std::make_unique<KeepAliveMsgSender>(uid, fd);
    }
  }
  // 删除对应的保活连接的Sender
  void delKpAliveSender(const int &uid) {
    std::lock_guard<std::mutex> lock(this->_keepalive_sender_mp_mu);
    LOG(INFO) << "delete keepalive sender for user " << uid;
    if (this->server_ptr_->_keepalive_sender_mp.find(uid) !=
        this->server_ptr_->_keepalive_sender_mp.end()) {
      this->server_ptr_->_keepalive_sender_mp.erase(uid);
    }
  }

  // id->sender
  KeepAliveMsgSender *getSender(const int &id) {
    if (id == 0) return nullptr;
    std::lock_guard<std::mutex> lock(this->_keepalive_sender_mp_mu);

    if (this->server_ptr_->_keepalive_sender_mp.find(id) !=
        this->server_ptr_->_keepalive_sender_mp.end()) {
      return this->server_ptr_->_keepalive_sender_mp[id].get();
    } else {
      return nullptr;
    }
  }
  // get token->uid
  int getUserIdByToken(const std::string &token) {
    if (token == "") return 0;
    std::lock_guard<std::mutex> lock(this->_token_pool_mu);
    if (this->server_ptr_->_token_pool.find(token) !=
        this->server_ptr_->_token_pool.end()) {
      return this->server_ptr_->_token_pool[token];
    } else {
      return 0;
    }
  }
  // set token->uid
  void setUserIdByToken(const std::string &token, const int &uid) {
    std::lock_guard<std::mutex> lock(this->_token_pool_mu);
    this->server_ptr_->_token_pool[token] = uid;
  }

  // 用户是否在线
  bool isOnline(const int uid) {
    KeepAliveMsgSender *sender = getSender(uid);
    if (sender)
      return true;
    else
      return false;
  }

  //文件消息转发
  void setFileMsg(const std::string &fileid, const int &uid,
                  const netdesign2::Message &msg) {
    std::lock_guard<std::mutex> lock(this->_file_msg_pool_mu);
    this->server_ptr_->_file_msg_pool[fileid] = std::make_pair(uid, std::move(msg));
  }

  void delFileMsg(const std::string &fileid) {
    std::lock_guard<std::mutex> lock(this->_file_msg_pool_mu);
    if (this->server_ptr_->_file_msg_pool.find(fileid) !=
        this->server_ptr_->_file_msg_pool.end()) {
      this->server_ptr_->_file_msg_pool.erase(fileid);
    }
  }

  std::pair<int, netdesign2::Message> getFileMsg(const std::string &fileid) {
    std::lock_guard<std::mutex> lock(this->_file_msg_pool_mu);
    if (this->server_ptr_->_file_msg_pool.find(fileid) !=
        this->server_ptr_->_file_msg_pool.end()) {
      return this->server_ptr_->_file_msg_pool[fileid];
    } else {
      return std::make_pair(0, netdesign2::Message());
    }
  }

};
}  // namespace ntc
#endif  //_USER_MANNAER_H
