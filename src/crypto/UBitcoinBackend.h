#ifndef NOSTR_UBITCOIN_BACKEND_H
#define NOSTR_UBITCOIN_BACKEND_H

#include <stddef.h>
#include <stdint.h>
#include <functional>

namespace nostr {
namespace crypto {

class UBitcoinBackend {
  public:
    using RandomProvider = std::function<bool(uint8_t *, size_t)>;

    static bool initialize(const RandomProvider &randomProvider);
    static void shutdown();
    static bool isInitialized();
    static bool publicKey(const uint8_t privateKey[32], uint8_t xOnlyPublicKey[32]);
    static bool sign(const uint8_t privateKey[32], const uint8_t *message, size_t messageSize,
                     const uint8_t auxiliaryRandomness[32], uint8_t signature[64]);
    static bool verify(const uint8_t xOnlyPublicKey[32], const uint8_t *message, size_t messageSize,
                       const uint8_t signature[64]);
    static bool ecdhRawX(const uint8_t privateKey[32], const uint8_t xOnlyPublicKey[32],
                         uint8_t sharedX[32]);
};

} // namespace crypto
} // namespace nostr

#endif
