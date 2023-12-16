#include "db_access.h"

namespace ntc {

[[nodiscard]] std::optional<User> DataAccess::getUser(
    std::string_view username) {
  SQLite::Statement query(db_, "SELECT * FROM users WHERE username = ?");
  query.bind(1, username.data());
  if (query.executeStep()) {
    User u(query.getColumn("user_id").getInt(),
           query.getColumn("username").getString(),
           query.getColumn("password").getString());
    return u;
  }
  return {};
}

[[nodiscard]] std::optional<User> DataAccess::getUser(const int &id) {
  SQLite::Statement query(db_, "SELECT * FROM users WHERE user_id = ?");
  query.bind(1, id);
  if (query.executeStep()) {
    User u(query.getColumn("user_id").getInt(),
           query.getColumn("username").getString(),
           query.getColumn("password").getString());
    return u;
  }
  return {};
}

[[nodiscard]] std::optional<Group> DataAccess::getGroup(std::string_view name) {
  SQLite::Statement query(db_, "SELECT * FROM groups WHERE group_name = ?");
  query.bind(1, name.data());
  if (query.executeStep()) {
    Group g(query.getColumn("group_id").getInt(),
            query.getColumn("group_name").getString());
    return g;
  }
  return {};
}

[[nodiscard]] std::optional<Group> DataAccess::getGroup(const int &id) {
  SQLite::Statement query(db_, "SELECT * FROM groups WHERE group_id = ?");
  query.bind(1, id);
  if (query.executeStep()) {
    Group g(query.getColumn("group_id").getInt(),
            query.getColumn("group_name").getString());
    return g;
  }
  return {};
}

}  // namespace ntc
