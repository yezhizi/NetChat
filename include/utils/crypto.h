#ifndef CRYPTO_UTILS_H
#define CRYPTO_UTILS_H

#include <cstdlib>
#include <ctime>

#include "base.h"
#include "sodium.h"
#include "utils/misc.h"
#include "utils/logging.h"

namespace utils::crypto {

inline static void InitSodium() {
  if (sodium_init() < 0) {
    LOG(FATAL) << "libsodium init failed";
  }
}

inline std::string bytesToHexString(const unsigned char *bytes, size_t length) {
  char hexString[length * 2 + 1];  // Each byte is represented by 2 characters,
                                   // plus 1 for null terminator
  for (size_t i = 0; i < length; ++i) {
    sprintf(&hexString[i * 2], "%02x", bytes[i]);
  }
  return std::string(hexString);
}

inline std::string genChallenge() {
  // 设置随机种子
  std::srand(static_cast<unsigned int>(std::time(nullptr)));

  const char charset[] =
      "0123456789"
      "abcdefghijklmnopqrstuvwxyz"
      "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

  const int charsetSize = sizeof(charset) - 1;

  std::string randomString;
  randomString.reserve(ntc::kChallengeSize);

  for (int i = 0; i < ntc::kChallengeSize; ++i) {
    randomString += charset[std::rand() % charsetSize];
  }

  return randomString;
}

// SHA256
inline std::string SHA256(const unsigned char *in, unsigned long long inlen) {
  InitSodium();
  unsigned char out[crypto_hash_sha256_BYTES];
  int ret = crypto_hash_sha256(out, in, inlen);
  if (ret != 0) {
    LOG(ERROR) << "SHA256 failed";
    return "";
  }
  return bytesToHexString(out, crypto_hash_sha256_BYTES);
}

inline std::string SHA256(const std::string &in) {
  return SHA256((const unsigned char *)in.c_str(), in.size());
}

// 生成uuid作为token
inline std::string genToken() {
  InitSodium();
  return utils::misc::getUUID();
}

}  // namespace utils::crypto

#endif  // CRYPTO_UTILS_H