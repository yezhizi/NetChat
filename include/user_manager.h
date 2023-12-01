#ifndef _USER_MANNAER_H
#define _USER_MANNAER_H
// TODO UM类，提供数据库接口，存放用户信息（账号、密码、昵称）、缓存消息
// 连接管理，用户与socket的映射,直接存内存
#include <string>
#include <memory>
namespace ntc {
class UM {
  public:
    static inline UM *getInstance() {
        return _getsharedPtr().get();
    }
    static inline std::shared_ptr<UM> getSharedPtr() {
        return _getsharedPtr();
    }
    /////////////////////////
    int getLongLifeSocket(const std::string &user_id) {
        // TODO 在表中找到对应的长连接socket
        return 0;
    }
    int setLongLifeSocket(const std::string &user_id, int fd) {
        // TODO 在表中设置对应的长连接socket
        return 0;
    }

  private:
    UM(){};
    UM(const UM &) = delete;
    UM &operator=(const UM &) = delete;
    static std::shared_ptr<UM> _getsharedPtr() {
        static std::shared_ptr<UM> instance_ptr(new UM());
        return instance_ptr;
    }
};
}
#endif //_USER_MANNAER_H