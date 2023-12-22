#ifndef MISC_H
#define MISC_H
#include <chrono>

#include "uuid/uuid.h"

namespace utils::misc {

// 获取当前时间戳, Unix timestamp in seconds
inline int getTimestamp() {
  return int(std::chrono::duration_cast<std::chrono::seconds>(
                 std::chrono::system_clock::now().time_since_epoch())
                 .count());
}

// 生成 UUID
inline std::string getUUID() {
  uuid_t uuid;
  uuid_generate_random(uuid);
  char str[37];
  uuid_unparse_lower(uuid, str);
  return std::string(str);
}

}  // namespace utils::misc

#endif  // MISC_H