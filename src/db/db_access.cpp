#include "db_access.h"

namespace ntc {

/* Users */

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

// Return with RVO
[[nodiscard]] std::vector<User> DataAccess::getAllUsers() {
  std::vector<User> users;
  SQLite::Statement query(db_, "SELECT * FROM users");
  while (query.executeStep()) {
    User u(query.getColumn("user_id").getInt(),
           query.getColumn("username").getString(),
           query.getColumn("password").getString());
    users.push_back(u);
  }
  return users;
}

/* Groups */

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

[[nodiscard]] std::vector<Group> DataAccess::getAllGroups() {
  std::vector<Group> groups;
  SQLite::Statement query(db_, "SELECT * FROM groups");
  while (query.executeStep()) {
    Group g(query.getColumn("group_id").getInt(),
            query.getColumn("group_name").getString());
    groups.push_back(g);
  }
  return groups;
}

/* Messages */

[[nodiscard]] std::optional<netdesign2::Message> DataAccess::getSavedMessage(
    const int &sender_id, const int &receiver_id, const int &internal_id) {
  SQLite::Statement query(db_,
                          "SELECT * FROM messages WHERE sender_id = ? "
                          "AND receiver_id = ? AND "
                          "internal_id = ?");
  query.bind(1, sender_id);
  query.bind(2, receiver_id);
  query.bind(3, internal_id);
  if (query.executeStep()) {
    netdesign2::Message m;

    m.mutable_message()->set_from(query.getColumn("sender_id").getInt());
    m.mutable_message()->set_to(query.getColumn("receiver_id").getInt());
    m.mutable_message()->set_content(query.getColumn("content").getString());
    m.mutable_message()->set_hash(query.getColumn("hash").getString());
    m.mutable_message()->set_type(
        static_cast<netdesign2::MessageType>(query.getColumn("type").getInt()));

    m.set_id(query.getColumn("message_id").getInt());
    m.set_internalid(query.getColumn("internal_id").getInt());
    m.set_timestamp(query.getColumn("timestamp").getInt());

    return m;
  }
  return {};
}

[[nodiscard]] std::optional<netdesign2::File> DataAccess::getFile(
    const int &id) {
  SQLite::Statement query(db_, "SELECT * FROM files WHERE file_id = ?");
  query.bind(1, id);
  if (query.executeStep()) {
    netdesign2::File f;
    // TODO: ID?
    // TODO: read content from disk?
    // TODO: hash? should from client side...
    f.set_name(query.getColumn("filename").getString());
    return f;
  }
  return {};
}

// Id is auto-incremented
bool DataAccess::createUser(const User &u) {
  SQLite::Statement query(db_,
                          "INSERT INTO users (username, password) VALUES "
                          "(?, ?)");
  query.bind(1, u.getUsername());
  query.bind(2, u.getPassword());
  return query.exec() == 1;
}

// Id is auto-incremented
bool DataAccess::createGroup(const Group &g) {
  SQLite::Statement query(db_, "INSERT INTO groups (group_name) VALUES (?)");
  query.bind(1, g.getName());
  return query.exec() == 1;
}

// Id is auto-incremented
bool DataAccess::createSavedMessage(const netdesign2::Message &m) {
  SQLite::Statement query(
      db_,
      "INSERT INTO messages (sender_id, receiver_id, content, hash, type, "
      "internal_id, timestamp) VALUES (?, ?, ?, ?, ?, ?, ?)");
  query.bind(1, m.message().from());
  query.bind(2, m.message().to());
  query.bind(3, m.message().content());
  query.bind(4, m.message().hash());
  query.bind(5, static_cast<int>(m.message().type()));
  query.bind(6, m.internalid());
  query.bind(7, m.timestamp());
  return query.exec() == 1;
}

// Id is auto-incremented
bool DataAccess::createFile(const netdesign2::File &f) {
  SQLite::Statement query(db_, "INSERT INTO files (filename) VALUES (?)");
  query.bind(1, f.name());
  return query.exec() == 1;
}

}  // namespace ntc
