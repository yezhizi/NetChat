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

inline void genChallenge(char *challenge) {
    InitSodium();
    if (challenge == nullptr) {
        LOG(FATAL) << "challenge buffer is nullptr";
    }
    randombytes_buf(challenge, ntc::kChallengeSize);
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
    return std::string((char *)out, crypto_hash_sha256_BYTES);
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