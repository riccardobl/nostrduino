#include "UBitcoinBackend.h"
#include "CryptoLock.h"
#include "../uBitcoin/src/Bitcoin.h"
#include "../uBitcoin/src/utility/trezor/ecdsa.h"
#include "../uBitcoin/src/utility/trezor/secp256k1.h"
#include "../uBitcoin/src/utility/trezor/sha2.h"
#include <cstring>
#include <mutex>

namespace {
nostr::crypto::UBitcoinBackend::RandomProvider randomProvider;
bool randomFailure = false;

const uint8_t curveOrder[32] = {
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe,
    0xba, 0xae, 0xdc, 0xe6, 0xaf, 0x48, 0xa0, 0x3b,
    0xbf, 0xd2, 0x5e, 0x8c, 0xd0, 0x36, 0x41, 0x41
};
void wipe(void *memory, size_t size) {
    if (memory == nullptr) return;
    volatile uint8_t *p = static_cast<volatile uint8_t *>(memory);
    while (size-- > 0) *p++ = 0;
}

bool isZero(const uint8_t value[32]) {
    uint8_t aggregate = 0;
    for (size_t i = 0; i < 32; ++i) aggregate |= value[i];
    return aggregate == 0;
}

bool lessThan(const uint8_t left[32], const uint8_t right[32]) {
    return std::memcmp(left, right, 32) < 0;
}

bool validSecret(const uint8_t privateKey[32]) {
    return privateKey != nullptr && !isZero(privateKey) && lessThan(privateKey, curveOrder);
}

bool beginOperation() {
    randomFailure = false;
    uint8_t probe[4] = {};
    if (!randomProvider || !randomProvider(probe, sizeof(probe))) {
        wipe(probe, sizeof(probe));
        randomFailure = true;
        return false;
    }
    wipe(probe, sizeof(probe));
    return true;
}

bool validFieldElement(const uint8_t value[32]) {
    // secp256k1's field prime is 2^256 - 2^32 - 977.
    static const uint8_t prime[32] = {
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xfe, 0xff, 0xff, 0xfc, 0x2f
    };
    return value != nullptr && lessThan(value, prime);
}

bool liftX(const uint8_t xOnly[32], PublicKey &publicKey) {
    if (!validFieldElement(xOnly)) return false;
    uint8_t compressed[33] = {};
    compressed[0] = 0x02;
    std::memcpy(compressed + 1, xOnly, 32);
    PublicKey parsed(compressed);
    wipe(compressed, sizeof(compressed));
    if (!parsed.isValid() || !parsed.isEven()) return false;
    publicKey = parsed;
    return true;
}

bool taggedHash(const char *tag, const uint8_t *first, size_t firstSize,
                const uint8_t *second, size_t secondSize,
                const uint8_t *third, size_t thirdSize, uint8_t output[32]) {
    if (tag == nullptr || output == nullptr || (firstSize != 0 && first == nullptr) ||
        (secondSize != 0 && second == nullptr) || (thirdSize != 0 && third == nullptr)) {
        wipe(output, 32);
        return false;
    }

    uint8_t tagHash[32] = {};
    SHA256_CTX tagContext;
    sha256_Init(&tagContext);
    sha256_Update(&tagContext, reinterpret_cast<const uint8_t *>(tag), std::strlen(tag));
    sha256_Final(&tagContext, tagHash);

    SHA256_CTX context;
    sha256_Init(&context);
    sha256_Update(&context, tagHash, sizeof(tagHash));
    sha256_Update(&context, tagHash, sizeof(tagHash));
    if (firstSize != 0) sha256_Update(&context, first, firstSize);
    if (secondSize != 0) sha256_Update(&context, second, secondSize);
    if (thirdSize != 0) sha256_Update(&context, third, thirdSize);
    sha256_Final(&context, output);
    wipe(tagHash, sizeof(tagHash));
    return true;
}
}

extern "C" uint32_t random32(void) {
    uint32_t value = 0;
    if (!randomProvider || !randomProvider(reinterpret_cast<uint8_t *>(&value), sizeof(value))) {
        value = 0;
        randomFailure = true;
    }
    return value;
}

bool nostr::crypto::UBitcoinBackend::initialize(const RandomProvider &provider) {
    std::lock_guard<std::mutex> lock(uBitcoinMutex());
    randomProvider = provider;
    randomFailure = false;
    return static_cast<bool>(randomProvider);
}

void nostr::crypto::UBitcoinBackend::shutdown() {
    std::lock_guard<std::mutex> lock(uBitcoinMutex());
    randomProvider = RandomProvider();
    randomFailure = false;
}

bool nostr::crypto::UBitcoinBackend::isInitialized() {
    std::lock_guard<std::mutex> lock(uBitcoinMutex());
    return static_cast<bool>(randomProvider);
}

bool nostr::crypto::UBitcoinBackend::publicKey(const uint8_t privateKey[32], uint8_t xOnlyPublicKey[32]) {
    std::lock_guard<std::mutex> lock(uBitcoinMutex());
    if (xOnlyPublicKey == nullptr) return false;
    wipe(xOnlyPublicKey, 32);
    if (!validSecret(privateKey) || !beginOperation()) return false;
    PrivateKey key(privateKey);
    PublicKey publicKey = key.publicKey();
    if (randomFailure || !publicKey.isValid() || publicKey.x(xOnlyPublicKey, 32) != 32) {
        wipe(xOnlyPublicKey, 32);
        return false;
    }
    return true;
}

bool nostr::crypto::UBitcoinBackend::sign(const uint8_t privateKey[32], const uint8_t *message,
                                          size_t messageSize, const uint8_t auxiliaryRandomness[32],
                                          uint8_t signature[64]) {
    std::lock_guard<std::mutex> lock(uBitcoinMutex());
    if (signature == nullptr) return false;
    wipe(signature, 64);
    if (!validSecret(privateKey) || (message == nullptr && messageSize != 0) ||
        auxiliaryRandomness == nullptr || !beginOperation()) return false;

    uint8_t adjustedSecretBytes[32] = {};
    uint8_t publicKeyBytes[32] = {};
    uint8_t auxiliaryHash[32] = {};
    uint8_t nonceInput[32] = {};
    uint8_t nonceHash[32] = {};
    uint8_t challengeHash[32] = {};
    bool result = false;

    PrivateKey secret(privateKey);
    PublicKey publicKey = secret.publicKey();
    PrivateKey adjustedSecret = publicKey.isEven() ? secret : PrivateKey(-secret);
    publicKey = adjustedSecret.publicKey();
    adjustedSecret.getSecret(adjustedSecretBytes);

    if (!publicKey.isValid() || publicKey.x(publicKeyBytes, sizeof(publicKeyBytes)) != 32 ||
        !taggedHash("BIP0340/aux", auxiliaryRandomness, 32, nullptr, 0, nullptr, 0, auxiliaryHash)) goto cleanup;
    for (size_t i = 0; i < 32; ++i) nonceInput[i] = adjustedSecretBytes[i] ^ auxiliaryHash[i];
    if (!taggedHash("BIP0340/nonce", nonceInput, 32, publicKeyBytes, 32,
                    message, messageSize, nonceHash) || isZero(nonceHash)) goto cleanup;

    {
        ECScalar nonceScalar(nonceHash, 32);
        if (!nonceScalar.isValid()) goto cleanup;
        PrivateKey nonce(nonceScalar);
        PublicKey noncePoint = nonce.publicKey();
        if (!noncePoint.isEven()) {
            nonce = PrivateKey(-nonce);
            noncePoint = nonce.publicKey();
        }
        if (!noncePoint.isValid() || noncePoint.x(signature, 32) != 32 ||
            !taggedHash("BIP0340/challenge", signature, 32, publicKeyBytes, 32,
                        message, messageSize, challengeHash)) goto cleanup;

        ECScalar challenge(challengeHash, 32);
        ECScalar response = nonce + challenge * adjustedSecret;
        response.getSecret(signature + 32);
        result = !randomFailure && lessThan(signature + 32, curveOrder);
    }

cleanup:
    if (!result) wipe(signature, 64);
    wipe(adjustedSecretBytes, sizeof(adjustedSecretBytes));
    wipe(publicKeyBytes, sizeof(publicKeyBytes));
    wipe(auxiliaryHash, sizeof(auxiliaryHash));
    wipe(nonceInput, sizeof(nonceInput));
    wipe(nonceHash, sizeof(nonceHash));
    wipe(challengeHash, sizeof(challengeHash));
    return result;
}

bool nostr::crypto::UBitcoinBackend::verify(const uint8_t xOnlyPublicKey[32], const uint8_t *message,
                                            size_t messageSize, const uint8_t signature[64]) {
    std::lock_guard<std::mutex> lock(uBitcoinMutex());
    if (xOnlyPublicKey == nullptr || (message == nullptr && messageSize != 0) || signature == nullptr ||
        !validFieldElement(signature) || !lessThan(signature + 32, curveOrder) || !beginOperation()) return false;

    PublicKey publicKey;
    if (!liftX(xOnlyPublicKey, publicKey)) return false;

    uint8_t challengeHash[32] = {};
    uint8_t candidateX[32] = {};
    if (!taggedHash("BIP0340/challenge", signature, 32, xOnlyPublicKey, 32,
                    message, messageSize, challengeHash)) return false;

    ECScalar response(signature + 32, 32);
    ECScalar challenge(challengeHash, 32);
    ECPoint responsePoint = response.isValid() ? response * GeneratorPoint : InfinityPoint;
    ECPoint challengePoint = challenge.isValid() ? challenge * publicKey : InfinityPoint;
    ECPoint noncePoint = responsePoint - challengePoint;
    const bool result = !randomFailure && noncePoint != InfinityPoint && noncePoint.isValid() && noncePoint.isEven() &&
                        noncePoint.x(candidateX, sizeof(candidateX)) == 32 &&
                        std::memcmp(candidateX, signature, 32) == 0;
    wipe(challengeHash, sizeof(challengeHash));
    wipe(candidateX, sizeof(candidateX));
    return result;
}

bool nostr::crypto::UBitcoinBackend::verifyRecoverableEcdsa(
    const uint8_t digest[32], const uint8_t signature[64], uint8_t recoveryId,
    const uint8_t compressedPublicKey[33]) {
    std::lock_guard<std::mutex> lock(uBitcoinMutex());
    static const uint8_t halfCurveOrder[32] = {
        0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0x5d, 0x57, 0x6e, 0x73, 0x57, 0xa4, 0x50, 0x1d,
        0xdf, 0xe9, 0x2f, 0x46, 0x68, 0x1b, 0x20, 0xa0
    };
    if (digest == nullptr || signature == nullptr || recoveryId > 3 ||
        (compressedPublicKey != nullptr && std::memcmp(signature + 32, halfCurveOrder, 32) > 0) ||
        !beginOperation()) return false;
    uint8_t recovered[65] = {};
    bool result = ecdsa_recover_pub_from_sig(&secp256k1, recovered, signature, digest,
                                             recoveryId) == 0 &&
                  ecdsa_verify_digest(&secp256k1, recovered, signature, digest) == 0 &&
                  !randomFailure;
    if (result && compressedPublicKey != nullptr) {
        result = (compressedPublicKey[0] == 0x02 || compressedPublicKey[0] == 0x03) &&
                 std::memcmp(compressedPublicKey + 1, recovered + 1, 32) == 0 &&
                 ((recovered[64] & 1) == (compressedPublicKey[0] & 1));
    }
    wipe(recovered, sizeof(recovered));
    return result;
}

bool nostr::crypto::UBitcoinBackend::ecdhRawX(const uint8_t privateKey[32],
                                              const uint8_t xOnlyPublicKey[32], uint8_t sharedX[32]) {
    std::lock_guard<std::mutex> lock(uBitcoinMutex());
    if (sharedX == nullptr) return false;
    wipe(sharedX, 32);
    if (!validSecret(privateKey) || xOnlyPublicKey == nullptr || !beginOperation()) return false;
    PublicKey publicKey;
    if (!liftX(xOnlyPublicKey, publicKey)) {
        wipe(sharedX, 32);
        return false;
    }
    PrivateKey secret(privateKey);
    if (secret.ecdh(publicKey, sharedX, false) != 1 || randomFailure) {
        wipe(sharedX, 32);
        return false;
    }
    return true;
}
