#ifndef DB_H
#define DB_H

#include <SQLiteCpp/SQLiteCpp.h>

namespace ntc {

class DataAccess {
 private:
  SQLite::Database db;

 public:
  // 构造函数
  DataAccess(const std::string& dbPath = "chat.db")
      : db(dbPath, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE) {
    // 打开数据库连接
    // 检查并创建表
    checkAndCreateTables();
  }

  // 析构函数
  ~DataAccess() {
    // SQLiteCpp会自动关闭数据库连接
  }

  // 检查并创建表
  void checkAndCreateTables() {
    // 检查并创建用户表
    if (!db.tableExists("users")) {
      db.exec(
          "CREATE TABLE users ("
          "user_id INTEGER PRIMARY KEY AUTOINCREMENT, "
          "username TEXT NOT NULL UNIQUE, "
          "display_name TEXT, "
          "email TEXT, "
          "phone TEXT)");
    }

    // 检查并创建群组表
    if (!db.tableExists("groups")) {
      db.exec(
          "CREATE TABLE groups ("
          "group_id INTEGER PRIMARY KEY AUTOINCREMENT, "
          "group_name TEXT NOT NULL, "
          "description TEXT)");
    }

    // 检查并创建用户-群组关联表
    if (!db.tableExists("user_group")) {
      db.exec(
          "CREATE TABLE user_group ("
          "user_id INTEGER, "
          "group_id INTEGER, "
          "FOREIGN KEY (user_id) REFERENCES users (user_id), "
          "FOREIGN KEY (group_id) REFERENCES groups (group_id), "
          "PRIMARY KEY (user_id, group_id))");
    }
  }

  // 其他成员函数...
};

}  // namespace ntc

#endif  // DB_H