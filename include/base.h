#ifndef _BASE_H
#define _BASE_H
#include <string>
namespace ntc {
static const int kvEmpty = 0;

static const int kMaxMessageSize = 1024 * 1024; // 1MB

static const std::string kServerIP = "127.0.0.1";
static const int kServerPort = 8081;

static const int ServerID = 1;

// 临时连接池工作线程的数量
static const int revc_socket_pool_thread_num = 10;

struct User {
    int id;
    std::string name;
    std::string phone_number;
    std::string password;
};
} // namespace ntc

#endif //_BASE_H