#ifndef DB_H
#define DB_H

#include <SQLiteCpp/SQLiteCpp.h>

#include <chrono>
#include <memory>
#include <optional>
#include <stdexcept>

#include "group.h"
#include "proto/messages.pb.h"
#include "user.h"
#include "utils/logging.h"
#include "utils/misc.h"

namespace ntc {
/* 数据库访问类 */
class DataAccess {
 public:
  // 构造函数，自动创建数据库和相关结构
  DataAccess(const std::string &dbPath = "chat.db")
      : db_(dbPath, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE) {
    LOG(INFO) << "Checking and creating DB...";
    checkAndCreateTables();
  }

  // 禁止拷贝和赋值
  DataAccess(const DataAccess &) = delete;
  DataAccess &operator=(const DataAccess &) = delete;

  // 析构函数
  ~DataAccess() {
    // SQLiteCpp会自动关闭数据库连接
  }

  [[nodiscard]] std::optional<User> getUser(std::string_view username);

  [[nodiscard]] std::optional<User> getUser(const int &id);

  [[nodiscard]] std::vector<User> getAllUsers();

  [[nodiscard]] std::optional<Group> getGroup(std::string_view name);

  [[nodiscard]] std::optional<Group> getGroup(const int &id);

  [[nodiscard]] std::vector<Group> getAllGroups();

  // Not implemented
  [[nodiscard]] std::vector<Group> getUserGroups(const int &user_id);

  // Not implemented
  [[nodiscard]] std::vector<User> getGroupUsers(const int &group_id);

  [[nodiscard]] std::optional<netdesign2::Message> getMessage(
      const int &sender_id, const int &receiver_id, const int &internal_id);

  [[nodiscard]] std::vector<netdesign2::Message> getAllMessages(
      const int &sender_id, const int &receiver_id);

  [[nodiscard]] std::optional<netdesign2::File> getFile(const int &id);

  bool createUser(const User &u);

  bool createGroup(const Group &g);

  // Create a message in DB and return its id
  std::optional<int> createMsg(const netdesign2::Message &m);

  // Create a message in DB by raw message
  std::optional<netdesign2::Message> createMsgByRawMsg(const netdesign2::RawMessage &m);

  bool createFile(const netdesign2::File &f);

 private:
  SQLite::Database db_;

  // 检查并创建表
  void checkAndCreateTables() {
    // 检查并创建用户表
    if (!db_.tableExists("users")) {
      db_.exec(
          "CREATE TABLE users ("
          "user_id INTEGER PRIMARY KEY AUTOINCREMENT, "
          "username TEXT NOT NULL UNIQUE, "
          "password TEXT NOT NULL)");
      LOG(DEBUG) << "`users` table created.";
    }

    // 检查并创建群组表
    if (!db_.tableExists("groups")) {
      db_.exec(
          "CREATE TABLE groups ("
          "group_id INTEGER PRIMARY KEY AUTOINCREMENT, "
          "group_name TEXT NOT NULL)");
      LOG(DEBUG) << "`groups` table created.";
    }

    // 检查并创建用户-群组关联表
    if (!db_.tableExists("user_group")) {
      db_.exec(
          "CREATE TABLE user_group ("
          "user_id INTEGER, "
          "group_id INTEGER, "
          "FOREIGN KEY (user_id) REFERENCES users (user_id), "
          "FOREIGN KEY (group_id) REFERENCES groups (group_id), "
          "PRIMARY KEY (user_id, group_id))");
      LOG(DEBUG) << "`user_group` table created.";
    }

    // 检查并创建文件表
    if (!db_.tableExists("files")) {
      db_.exec(
          "CREATE TABLE files ("
          "file_id INTEGER PRIMARY KEY AUTOINCREMENT, "
          "filename TEXT NOT NULL, "
          "hash TEXT NOT NULL, "
          "timestamp INTEGER NOT NULL)");
      LOG(DEBUG) << "`files` table created.";
    }

    // 检查并创建聊天记录表
    if (!db_.tableExists("messages")) {
      db_.exec(
          "CREATE TABLE messages ("
          "message_id INTEGER PRIMARY KEY AUTOINCREMENT, "
          "sender_id INTEGER NOT NULL, "
          "receiver_id INTEGER NOT NULL, "
          "type INTEGER NOT NULL, "  // oneof TEXT, IMAGE or FILE
          "internal_id INTEGER, "    // 用于标识对话内部的消息
          "content TEXT, "           // TEXT/IMAGE 类型的消息内容
          "file_id INTEGER, "        // FILE 类型外键
          "timestamp INTEGER NOT NULL, "
          "FOREIGN KEY (sender_id) REFERENCES users (user_id), "
          "FOREIGN KEY (receiver_id) REFERENCES users (user_id), "
          "FOREIGN KEY (file_id) REFERENCES files (file_id))");
      LOG(DEBUG) << "`messages` table created.";
    }
  }
};

// 全局的数据库操作对象，由于初始化需要参数，不适用 Get 形式的单例模式
// 需要保证在 main 函数中初始化
extern std::unique_ptr<DataAccess> g_db;

}  // namespace ntc

#endif  // DB_H
