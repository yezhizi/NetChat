#ifndef DB_H
#define DB_H

#include <SQLiteCpp/SQLiteCpp.h>

namespace ntc {
/* 数据库访问类 */
class DataAccess {
 private:
  SQLite::Database db;

 public:
  // 构造函数，自动创建数据库和相关结构
  DataAccess(const std::string& dbPath = "chat.db")
      : db(dbPath, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE) {
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
          "password TEXT NOT NULL)");
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

    // 检查并创建文件表
    if (!db.tableExists("files")) {
      db.exec(
          "CREATE TABLE files ("
          "file_id INTEGER PRIMARY KEY AUTOINCREMENT, "
          "filename TEXT NOT NULL, "
          "hash TEXT NOT NULL, "
          "timestamp INTEGER NOT NULL)");
    }

    // 检查并创建聊天记录表
    if (!db.tableExists("messages")) {
      db.exec(
          "CREATE TABLE messages ("
          "message_id INTEGER PRIMARY KEY AUTOINCREMENT, "
          "sender_id INTEGER NOT NULL, "
          "receiver_id INTEGER NOT NULL, "
          "type INTEGER NOT NULL, "  // oneof TEXT, IMAGE or FILE
          "internal_id INTEGER, "    // 用于标识对话内部的消息
          "content TEXT NOT NULL, "  // TEXT/IMAGE 类型的消息内容
          "file_id INTEGER, "        // FILE 类型外键
          "hash TEXT NOT NULL, "
          "timestamp INTEGER NOT NULL, "
          "FOREIGN KEY (sender_id) REFERENCES users (user_id), "
          "FOREIGN KEY (receiver_id) REFERENCES users (user_id), "
          "FOREIGN KEY (file_id) REFERENCES files (file_id))");
    }
  }
};

DataAccess* Db;  // 全局数据库访问对象

}  // namespace ntc

#endif  // DB_H