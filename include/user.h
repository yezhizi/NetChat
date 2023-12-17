#ifndef USER_H
#define USER_H

#include "base.h"

namespace ntc {

class User {
 private:
  int id_;
  std::string username_;
  std::string password_;

 public:
  User() : id_{0}, username_{""}, password_{""} {};
  User(int id, std::string_view username, std::string_view password)
      : id_{id}, username_{username.data()}, password_{password.data()} {}
  ~User() {}

  // 获取用户信息的方法
  int getId() const { return id_; };
  std::string getUsername() const { return username_; };
  std::string getPassword() const { return password_; };

  // 设置用户信息的方法
  void setId(int id) { id_ = id; };
  void setUsername(std::string_view username) {
    username_ = std::string{username};
  };
  void setPassword(std::string_view password) {
    password_ = std::string{password};
  };
};

}  // namespace ntc

#endif  // USER_H