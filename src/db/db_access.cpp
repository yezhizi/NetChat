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

[[nodiscard]] std::optional<netdesign2::Message> DataAccess::getMessage(
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
    m.mutable_message()->set_type(
        static_cast<netdesign2::MessageType>(query.getColumn("type").getInt()));

    m.set_id(query.getColumn("message_id").getInt());
    m.set_internalid(query.getColumn("internal_id").getInt());
    m.set_timestamp(query.getColumn("timestamp").getInt());

    return m;
  }
  return {};
}

// Get all messages betweet sender and recver
[[nodiscard]] std::vector<netdesign2::Message> DataAccess::getAllMessages(
    const int &sender_id, const int &receiver_id) {
  std::vector<netdesign2::Message> msgs;
  SQLite::Statement query(db_,
                          "SELECT * FROM messages WHERE sender_id = ? "
                          "AND receiver_id = ?");
  query.bind(1, sender_id);
  query.bind(2, receiver_id);
  while (query.executeStep()) {
    netdesign2::Message m;

    m.mutable_message()->set_from(query.getColumn("sender_id").getInt());
    m.mutable_message()->set_to(query.getColumn("receiver_id").getInt());
    m.mutable_message()->set_content(query.getColumn("content").getString());
    m.mutable_message()->set_type(
        static_cast<netdesign2::MessageType>(query.getColumn("type").getInt()));

    m.set_id(query.getColumn("message_id").getInt());
    m.set_internalid(query.getColumn("internal_id").getInt());
    m.set_timestamp(query.getColumn("timestamp").getInt());

    msgs.push_back(m);
  }

  return msgs;
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
  auto n = u.getUsername();
  auto p = u.getPassword();
  bool ret = false;

  try {
    SQLite::Statement query(db_,
                            "INSERT INTO users (username, password) VALUES "
                            "(?, ?)");
    query.bind(1, n);
    query.bind(2, p);
    ret = query.exec();
  } catch (const std::exception &e) {
    LOG(ERROR) << "Exception: " << e.what();
  }

  return ret;
}

// Id is auto-incremented
bool DataAccess::createGroup(const Group &g) {
  SQLite::Statement query(db_, "INSERT INTO groups (group_name) VALUES (?)");
  query.bind(1, g.getName());
  return query.exec() == 1;
}

std::optional<int> DataAccess::createMsg(const netdesign2::Message &m) {
  // Id is auto-incremented
  SQLite::Statement query(
      db_,
      "INSERT INTO messages (sender_id, receiver_id, content, type, "
      "internal_id, timestamp) VALUES (?, ?, ?, ?, ?, ?)");
  query.bind(1, m.message().from());
  query.bind(2, m.message().to());
  query.bind(3, m.message().content());
  query.bind(4, static_cast<int>(m.message().type()));
  query.bind(5, m.internalid());
  query.bind(6, m.timestamp());
  if (query.exec() == 1) {
    return db_.getLastInsertRowid();
  }
  return {};
}

std::optional<netdesign2::Message> DataAccess::createMsgByRawMsg(
    const netdesign2::RawMessage &m) {
  // make a message
  netdesign2::Message msg;
  msg.mutable_message()->set_from(m.from());
  msg.mutable_message()->set_to(m.to());
  msg.mutable_message()->set_content(m.content());
  msg.mutable_message()->set_type(m.type());

  // get existing msgs between sender and receiver
  // find the max internal id
  auto msgs = getAllMessages(m.from(), m.to());
  int max_internal_id = 0;
  for (const auto &msg : msgs) {
    if (msg.internalid() > max_internal_id) {
      max_internal_id = msg.internalid();
    }
  }
  msg.set_internalid(max_internal_id + 1);

  // set current unix timestamp
  msg.set_timestamp(utils::misc::getTimestamp());

  // save to db and get the id
  auto result = createMsg(msg);
  if (result.has_value()) {
    // set id and return
    msg.set_id(result.value());
    return msg;
  }

  return {};
}

// Id is auto-incremented
bool DataAccess::createFile(const netdesign2::File &f) {
  SQLite::Statement query(db_, "INSERT INTO files (filename) VALUES (?)");
  query.bind(1, f.name());
  return query.exec() == 1;
}

}  // namespace ntc
