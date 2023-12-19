#include <chrono>

namespace utils::misc {

// 获取当前时间戳, Unix timestamp in seconds
inline int getTimestamp() {
  return int(std::chrono::duration_cast<std::chrono::seconds>(
                 std::chrono::system_clock::now().time_since_epoch())
                 .count());
}

}  // namespace utils::misc
