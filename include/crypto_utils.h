#ifndef CRYPTO_UTILS_H
#define CRYPTO_UTILS_H

#include "base.h"
#include "logging.h"
#include "sodium.h"
#include <string>
#include <uuid/uuid.h>

namespace ntc {
inline static void InitSodium() {
    if (sodium_init() < 0) {
        LOG(FATAL) << "libsodium init failed";
    }
}
inline std::string bytesToHexString(const unsigned char *bytes, size_t length) {
    char hexString[length * 2 + 1];  // Each byte is represented by 2 characters, plus 1 for null terminator
    for (size_t i = 0; i < length; ++i) {
        sprintf(&hexString[i * 2], "%02x", bytes[i]);
    }
    return std::string(hexString);
}

inline std::string genChallenge() {
    InitSodium();
    char challenge[ntc::kChallengeSize];

    randombytes_buf(challenge, ntc::kChallengeSize);

    // format challenge to hex string
    std::string res =  bytesToHexString((const unsigned char *)challenge, ntc::kChallengeSize);
    LOG(DEBUG ) << "genChallenge: " << res;
    return res;
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
    uuid_t uu;
    uuid_generate(uu);

    char uuidStr[37]; // 36字符UUID + 1字符空终止符
    uuid_unparse(uu, uuidStr);

    return std::string(uuidStr);
}

} // namespace ntc

#endif // CRYPTO_UTILS_H