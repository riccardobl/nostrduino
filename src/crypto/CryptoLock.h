#ifndef NOSTR_CRYPTO_LOCK_H
#define NOSTR_CRYPTO_LOCK_H

#include <mutex>

namespace nostr {
namespace crypto {
std::mutex &uBitcoinMutex();
} // namespace crypto
} // namespace nostr

#endif
