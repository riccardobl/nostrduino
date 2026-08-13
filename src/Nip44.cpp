#include "Nip44.h"

#include "NostrEncoding.h"
#include "NostrUtils.h"
#include "crypto/UBitcoinBackend.h"
#include "crypto/CryptoLock.h"
#include "uBitcoin/src/utility/trezor/hmac.h"

#include <mbedtls/base64.h>
#include <cstring>
#include <limits>
#include <mutex>
#include <vector>

namespace {

using nostr::NostrLimits;

void wipe(void *memory, size_t size) {
    if (memory == nullptr) return;
    volatile uint8_t *p = static_cast<volatile uint8_t *>(memory);
    while (size-- > 0) *p++ = 0;
}

bool constantTimeEqual(const uint8_t *left, const uint8_t *right, size_t size) {
    if (left == nullptr || right == nullptr) return false;
    uint8_t difference = 0;
    for (size_t i = 0; i < size; ++i) difference |= left[i] ^ right[i];
    return difference == 0;
}

uint32_t load32Le(const uint8_t *value) {
    return static_cast<uint32_t>(value[0]) |
           (static_cast<uint32_t>(value[1]) << 8) |
           (static_cast<uint32_t>(value[2]) << 16) |
           (static_cast<uint32_t>(value[3]) << 24);
}

uint32_t load32Be(const uint8_t *value) {
    return (static_cast<uint32_t>(value[0]) << 24) |
           (static_cast<uint32_t>(value[1]) << 16) |
           (static_cast<uint32_t>(value[2]) << 8) |
           static_cast<uint32_t>(value[3]);
}

void store32Le(uint8_t *value, uint32_t number) {
    value[0] = static_cast<uint8_t>(number);
    value[1] = static_cast<uint8_t>(number >> 8);
    value[2] = static_cast<uint8_t>(number >> 16);
    value[3] = static_cast<uint8_t>(number >> 24);
}

void store32Be(uint8_t *value, uint32_t number) {
    value[0] = static_cast<uint8_t>(number >> 24);
    value[1] = static_cast<uint8_t>(number >> 16);
    value[2] = static_cast<uint8_t>(number >> 8);
    value[3] = static_cast<uint8_t>(number);
}

uint32_t rotateLeft(uint32_t value, uint32_t count) {
    return (value << count) | (value >> (32 - count));
}

void quarterRound(uint32_t &a, uint32_t &b, uint32_t &c, uint32_t &d) {
    a += b; d ^= a; d = rotateLeft(d, 16);
    c += d; b ^= c; b = rotateLeft(b, 12);
    a += b; d ^= a; d = rotateLeft(d, 8);
    c += d; b ^= c; b = rotateLeft(b, 7);
}

void chacha20Block(const uint8_t key[32], uint32_t counter, const uint8_t nonce[12],
                  uint8_t output[64]) {
    static const uint8_t sigma[] = "expand 32-byte k";
    uint32_t state[16] = {
        load32Le(sigma), load32Le(sigma + 4), load32Le(sigma + 8), load32Le(sigma + 12),
        load32Le(key), load32Le(key + 4), load32Le(key + 8), load32Le(key + 12),
        load32Le(key + 16), load32Le(key + 20), load32Le(key + 24), load32Le(key + 28),
        counter, load32Le(nonce), load32Le(nonce + 4), load32Le(nonce + 8)
    };
    uint32_t working[16];
    std::memcpy(working, state, sizeof(working));
    for (size_t round = 0; round < 10; ++round) {
        quarterRound(working[0], working[4], working[8], working[12]);
        quarterRound(working[1], working[5], working[9], working[13]);
        quarterRound(working[2], working[6], working[10], working[14]);
        quarterRound(working[3], working[7], working[11], working[15]);
        quarterRound(working[0], working[5], working[10], working[15]);
        quarterRound(working[1], working[6], working[11], working[12]);
        quarterRound(working[2], working[7], working[8], working[13]);
        quarterRound(working[3], working[4], working[9], working[14]);
    }
    for (size_t i = 0; i < 16; ++i) store32Le(output + i * 4, working[i] + state[i]);
    wipe(state, sizeof(state));
    wipe(working, sizeof(working));
}

void chacha20Xor(uint8_t *data, size_t dataSize, const uint8_t key[32], const uint8_t nonce[12]) {
    if (data == nullptr && dataSize != 0) return;
    uint8_t block[64] = {};
    uint32_t counter = 0;
    size_t offset = 0;
    while (offset < dataSize) {
        chacha20Block(key, counter++, nonce, block);
        const size_t chunk = (dataSize - offset < sizeof(block)) ? dataSize - offset : sizeof(block);
        for (size_t i = 0; i < chunk; ++i) data[offset + i] ^= block[i];
        offset += chunk;
    }
    wipe(block, sizeof(block));
}

bool hmacParts(const uint8_t *key, size_t keySize, const uint8_t *first, size_t firstSize,
               const uint8_t *second, size_t secondSize, uint8_t output[32]) {
    if (key == nullptr || output == nullptr || (first == nullptr && firstSize != 0) ||
        (second == nullptr && secondSize != 0) || firstSize > UINT32_MAX || secondSize > UINT32_MAX) {
        wipe(output, 32);
        return false;
    }
    std::lock_guard<std::mutex> lock(nostr::crypto::uBitcoinMutex());
    HMAC_SHA256_CTX context;
    ubtc_hmac_sha256_Init(&context, key, static_cast<uint32_t>(keySize));
    if (firstSize != 0) ubtc_hmac_sha256_Update(&context, first, static_cast<uint32_t>(firstSize));
    if (secondSize != 0) ubtc_hmac_sha256_Update(&context, second, static_cast<uint32_t>(secondSize));
    ubtc_hmac_sha256_Final(&context, output);
    return true;
}

bool hkdfExtract(const uint8_t *salt, size_t saltSize, const uint8_t *input, size_t inputSize,
                 uint8_t output[32]) {
    return hmacParts(salt, saltSize, input, inputSize, nullptr, 0, output);
}

bool hkdfExpand(const uint8_t prk[32], const uint8_t info[32], size_t outputSize, uint8_t *output) {
    if (prk == nullptr || info == nullptr || output == nullptr || outputSize == 0 || outputSize > 255 * 32) {
        wipe(output, outputSize);
        return false;
    }
    std::lock_guard<std::mutex> lock(nostr::crypto::uBitcoinMutex());
    uint8_t previous[32] = {};
    size_t previousSize = 0;
    size_t written = 0;
    uint8_t counter = 1;
    while (written < outputSize) {
        HMAC_SHA256_CTX context;
        ubtc_hmac_sha256_Init(&context, prk, 32);
        if (previousSize != 0) ubtc_hmac_sha256_Update(&context, previous, static_cast<uint32_t>(previousSize));
        ubtc_hmac_sha256_Update(&context, info, 32);
        ubtc_hmac_sha256_Update(&context, &counter, 1);
        ubtc_hmac_sha256_Final(&context, previous);
        const size_t chunk = (outputSize - written < sizeof(previous)) ? outputSize - written : sizeof(previous);
        std::memcpy(output + written, previous, chunk);
        written += chunk;
        ++counter;
        previousSize = sizeof(previous);
    }
    wipe(previous, sizeof(previous));
    return true;
}

bool validUtf8(const uint8_t *value, size_t valueSize) {
    if (value == nullptr && valueSize != 0) return false;
    size_t index = 0;
    while (index < valueSize) {
        const uint8_t first = value[index++];
        size_t continuationCount = 0;
        uint32_t codePoint = 0;
        uint32_t minimum = 0;
        if (first <= 0x7f) {
            continue;
        } else if (first >= 0xc2 && first <= 0xdf) {
            continuationCount = 1;
            codePoint = first & 0x1f;
            minimum = 0x80;
        } else if (first >= 0xe0 && first <= 0xef) {
            continuationCount = 2;
            codePoint = first & 0x0f;
            minimum = 0x800;
        } else if (first >= 0xf0 && first <= 0xf4) {
            continuationCount = 3;
            codePoint = first & 0x07;
            minimum = 0x10000;
        } else {
            return false;
        }
        if (index + continuationCount > valueSize) return false;
        for (size_t i = 0; i < continuationCount; ++i) {
            const uint8_t next = value[index++];
            if ((next & 0xc0) != 0x80) return false;
            codePoint = (codePoint << 6) | (next & 0x3f);
        }
        if (codePoint < minimum || codePoint > 0x10ffff ||
            (codePoint >= 0xd800 && codePoint <= 0xdfff)) return false;
    }
    return true;
}

size_t calcPaddedLength(size_t plaintextLength) {
    if (plaintextLength == 0) return 0;
    if (plaintextLength <= 32) return 32;
    size_t nextPower = 1;
    while (nextPower < plaintextLength) {
        if (nextPower > (SIZE_MAX / 2)) return 0;
        nextPower *= 2;
    }
    const size_t chunk = nextPower <= 256 ? 32 : nextPower / 8;
    const size_t chunks = (plaintextLength - 1) / chunk + 1;
    if (chunks > SIZE_MAX / chunk) return 0;
    return chunks * chunk;
}

bool deriveConversationKey(const uint8_t privateKey[32], const uint8_t publicKey[32], uint8_t output[32]) {
    static const uint8_t salt[] = {'n', 'i', 'p', '4', '4', '-', 'v', '2'};
    uint8_t sharedX[32] = {};
    const bool ok = nostr::crypto::UBitcoinBackend::ecdhRawX(privateKey, publicKey, sharedX) &&
                    hkdfExtract(salt, sizeof(salt), sharedX, sizeof(sharedX), output);
    wipe(sharedX, sizeof(sharedX));
    if (!ok) wipe(output, 32);
    return ok;
}

bool deriveMessageKeys(const uint8_t conversationKey[32], const uint8_t nonce[32],
                       uint8_t chachaKey[32], uint8_t chachaNonce[12], uint8_t hmacKey[32]) {
    uint8_t expanded[76] = {};
    if (!hkdfExpand(conversationKey, nonce, sizeof(expanded), expanded)) return false;
    std::memcpy(chachaKey, expanded, 32);
    std::memcpy(chachaNonce, expanded + 32, 12);
    std::memcpy(hmacKey, expanded + 44, 32);
    wipe(expanded, sizeof(expanded));
    return true;
}

bool padPlaintextInto(const uint8_t *plaintext, size_t plaintextSize, const NostrLimits &limits,
                      uint8_t *padded, size_t paddedCapacity, size_t &paddedSize) {
    paddedSize = 0;
    if (plaintext == nullptr || plaintextSize == 0 || plaintextSize > limits.maxNip44PlaintextBytes ||
        plaintextSize > UINT32_MAX || !validUtf8(plaintext, plaintextSize) || padded == nullptr) return false;
    const size_t paddedBodySize = calcPaddedLength(plaintextSize);
    const size_t prefixSize = plaintextSize < 65536 ? 2 : 6;
    if (paddedBodySize == 0 || paddedBodySize > SIZE_MAX - prefixSize ||
        prefixSize + paddedBodySize > paddedCapacity) return false;
    paddedSize = prefixSize + paddedBodySize;
    std::memset(padded, 0, paddedSize);
    if (prefixSize == 2) {
        padded[0] = static_cast<uint8_t>(plaintextSize >> 8);
        padded[1] = static_cast<uint8_t>(plaintextSize);
    } else {
        store32Be(padded + 2, static_cast<uint32_t>(plaintextSize));
    }
    std::memcpy(padded + prefixSize, plaintext, plaintextSize);
    return true;
}

bool paddedPlaintextRange(const uint8_t *padded, size_t paddedSize, const NostrLimits &limits,
                          size_t &prefixSize, size_t &plaintextSize) {
    if (padded == nullptr || paddedSize < 34) return false;
    prefixSize = 2;
    plaintextSize = (static_cast<size_t>(padded[0]) << 8) | padded[1];
    if (plaintextSize == 0) {
        if (paddedSize < 38) return false;
        plaintextSize = load32Be(padded + 2);
        prefixSize = 6;
        if (plaintextSize < 65536) return false;
    }
    if (plaintextSize == 0 || plaintextSize > limits.maxNip44PlaintextBytes ||
        plaintextSize > UINT32_MAX || prefixSize > paddedSize ||
        calcPaddedLength(plaintextSize) != paddedSize - prefixSize ||
        !validUtf8(padded + prefixSize, plaintextSize)) return false;
    return true;
}

bool unpadPlaintext(const uint8_t *padded, size_t paddedSize, const NostrLimits &limits, NostrString &plaintext) {
    size_t prefixSize = 0;
    size_t plaintextSize = 0;
    if (!paddedPlaintextRange(padded, paddedSize, limits, prefixSize, plaintextSize)) return false;
    try {
        plaintext = NostrString_fromBytes(reinterpret_cast<const char *>(padded + prefixSize), plaintextSize);
    } catch (...) {
        return false;
    }
    return true;
}

bool decodePayload(const NostrString &payload, const NostrLimits &limits, std::vector<uint8_t> &raw) {
    const long encodedLength = NostrString_length(payload);
    if (encodedLength < 132 || payload[0] == '#') return false;
    const size_t encodedSize = static_cast<size_t>(encodedLength);
    size_t decodedSize = 0;
    if (!nostr::validateBase64Encoding(payload.c_str(), encodedSize, false, false, decodedSize) ||
        decodedSize < 99 || decodedSize > limits.maxNip44PayloadBytes) return false;
    try {
        raw.assign(decodedSize, 0);
    } catch (...) {
        return false;
    }
    const size_t actualSize = nostr::Utils::fromBase64(payload.c_str(), encodedSize, raw.data(), raw.size(), BASE64_STANDARD);
    if (actualSize != decodedSize || raw[0] != 2) {
        wipe(raw.data(), raw.size());
        raw.clear();
        return false;
    }
    return true;
}

NostrString encodePayload(const std::vector<uint8_t> &raw) {
    if (raw.empty() || raw.size() > (SIZE_MAX - 2) / 4 * 3) return "";
    const size_t encodedSize = ((raw.size() + 2) / 3) * 4;
    if (encodedSize > UINT_MAX) return "";
    try {
        NostrString result;
        if (!result.reserve(static_cast<unsigned int>(encodedSize))) return "";
        static const char alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        char encoded[1024] = {};
        size_t inputOffset = 0;
        size_t outputSize = 0;
        while (inputOffset < raw.size()) {
            const size_t remaining = raw.size() - inputOffset;
            size_t inputSize = remaining < 768 ? remaining : 768;
            if (inputSize < remaining) inputSize -= inputSize % 3;
            const bool finalChunk = inputOffset + inputSize == raw.size();
            size_t encodedOffset = 0;
            for (size_t i = 0; i < inputSize; i += 3) {
                const size_t left = inputSize - i;
                const uint32_t value = (static_cast<uint32_t>(raw[inputOffset + i]) << 16) |
                                        (static_cast<uint32_t>(left > 1 ? raw[inputOffset + i + 1] : 0) << 8) |
                                        static_cast<uint32_t>(left > 2 ? raw[inputOffset + i + 2] : 0);
                encoded[encodedOffset++] = alphabet[(value >> 18) & 0x3f];
                encoded[encodedOffset++] = alphabet[(value >> 12) & 0x3f];
                encoded[encodedOffset++] = left > 1 ? alphabet[(value >> 6) & 0x3f] : '=';
                encoded[encodedOffset++] = left > 2 ? alphabet[value & 0x3f] : '=';
            }
            if (finalChunk && inputSize < remaining) {
                const size_t i = inputSize;
                const size_t left = remaining - inputSize;
                const uint32_t value = static_cast<uint32_t>(raw[inputOffset + i]) << 16 |
                                        (left > 1 ? static_cast<uint32_t>(raw[inputOffset + i + 1]) << 8 : 0);
                encoded[encodedOffset++] = alphabet[(value >> 18) & 0x3f];
                encoded[encodedOffset++] = alphabet[(value >> 12) & 0x3f];
                encoded[encodedOffset++] = left > 1 ? alphabet[(value >> 6) & 0x3f] : '=';
                encoded[encodedOffset++] = '=';
            }
            NostrString piece = NostrString_fromBytes(encoded, encodedOffset);
            result += piece;
            outputSize += encodedOffset;
            inputOffset += inputSize;
            if (finalChunk) break;
        }
        return outputSize == encodedSize && NostrString_length(result) == static_cast<long>(encodedSize) ? result : NostrString();
    } catch (...) {
        return "";
    }
}

class Base64Writer {
  public:
    Base64Writer(char *output, size_t capacity) : output(output), capacity(capacity), written(0), buffered(0) {}

    bool append(const uint8_t *bytes, size_t byteCount) {
        if (bytes == nullptr && byteCount != 0) return false;
        for (size_t i = 0; i < byteCount; ++i) {
            buffer[buffered++] = bytes[i];
            if (buffered == sizeof(buffer) && !emit(3)) return false;
        }
        return true;
    }

    bool finish(size_t &outputSize) {
        if (buffered != 0 && !emit(buffered)) return false;
        if (output == nullptr || written >= capacity) return false;
        output[written] = '\0';
        outputSize = written;
        return true;
    }

  private:
    bool emit(size_t inputSize) {
        static const char alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        if (output == nullptr || inputSize == 0 || inputSize > 3 || capacity < 4 || written > capacity - 4) return false;
        const uint32_t value = (static_cast<uint32_t>(buffer[0]) << 16) |
                               (static_cast<uint32_t>(inputSize > 1 ? buffer[1] : 0) << 8) |
                               static_cast<uint32_t>(inputSize > 2 ? buffer[2] : 0);
        output[written++] = alphabet[(value >> 18) & 0x3f];
        output[written++] = alphabet[(value >> 12) & 0x3f];
        output[written++] = inputSize > 1 ? alphabet[(value >> 6) & 0x3f] : '=';
        output[written++] = inputSize > 2 ? alphabet[value & 0x3f] : '=';
        buffered = 0;
        return true;
    }

    char *output;
    size_t capacity;
    size_t written;
    uint8_t buffered;
    uint8_t buffer[3] = {};
};

bool encryptBinary(uint8_t *plaintext, size_t plaintextSize, size_t plaintextBufferCapacity,
                   const uint8_t conversationKey[32], const uint8_t nonce[32], char *payloadBuffer,
                   size_t payloadCapacity, size_t &payloadSize) {
    payloadSize = 0;
    const NostrLimits limits;
    if (plaintext == nullptr || conversationKey == nullptr || nonce == nullptr || payloadBuffer == nullptr ||
        plaintextSize == 0 || plaintextSize > limits.maxNip44PlaintextBytes || plaintextSize > UINT32_MAX ||
        !validUtf8(plaintext, plaintextSize)) return false;
    const size_t paddedBodySize = calcPaddedLength(plaintextSize);
    const size_t prefixSize = plaintextSize < 65536 ? 2 : 6;
    if (paddedBodySize == 0 || paddedBodySize > SIZE_MAX - prefixSize ||
        prefixSize + paddedBodySize > plaintextBufferCapacity) return false;
    const size_t paddedSize = prefixSize + paddedBodySize;
    if (paddedSize > SIZE_MAX - 65) return false;
    const size_t rawSize = 1 + 32 + paddedSize + 32;
    if (rawSize > limits.maxNip44PayloadBytes || rawSize > (SIZE_MAX - 2) / 4 * 3) return false;
    const size_t encodedSize = ((rawSize + 2) / 3) * 4;
    if (encodedSize > SIZE_MAX - 1 || payloadCapacity < encodedSize + 1) return false;

    // The caller owns this workspace. Moving the plaintext in place keeps
    // large protocol vectors usable on ESP32 without a second plaintext copy.
    std::memmove(plaintext + prefixSize, plaintext, plaintextSize);
    uint8_t *ciphertext = plaintext;
    std::memset(ciphertext + prefixSize + plaintextSize, 0, paddedBodySize - plaintextSize);
    if (prefixSize == 2) {
        ciphertext[0] = static_cast<uint8_t>(plaintextSize >> 8);
        ciphertext[1] = static_cast<uint8_t>(plaintextSize);
    } else {
        ciphertext[0] = 0;
        ciphertext[1] = 0;
        store32Be(ciphertext + 2, static_cast<uint32_t>(plaintextSize));
    }

    uint8_t chachaKey[32] = {}, chachaNonce[12] = {}, hmacKey[32] = {}, mac[32] = {};
    bool ok = deriveMessageKeys(conversationKey, nonce, chachaKey, chachaNonce, hmacKey);
    if (ok) {
        chacha20Xor(ciphertext, paddedSize, chachaKey, chachaNonce);
        ok = hmacParts(hmacKey, sizeof(hmacKey), nonce, 32, ciphertext, paddedSize, mac);
    }
    if (ok) {
        const uint8_t version = 2;
        Base64Writer writer(payloadBuffer, payloadCapacity);
        ok = writer.append(&version, 1) && writer.append(nonce, 32) &&
             writer.append(ciphertext, paddedSize) && writer.append(mac, sizeof(mac)) &&
             writer.finish(payloadSize) && payloadSize == encodedSize;
    }
    wipe(chachaKey, sizeof(chachaKey)); wipe(chachaNonce, sizeof(chachaNonce));
    wipe(hmacKey, sizeof(hmacKey)); wipe(mac, sizeof(mac));
    if (!ok) {
        wipe(ciphertext, paddedSize);
        payloadSize = 0;
        if (payloadCapacity != 0) payloadBuffer[0] = '\0';
    }
    return ok;
}

bool decryptBinary(const char *payload, size_t payloadSize, const uint8_t conversationKey[32],
                   uint8_t *plaintextBuffer, size_t plaintextBufferCapacity, size_t &plaintextSize) {
    plaintextSize = 0;
    const NostrLimits limits;
    if (payload == nullptr || conversationKey == nullptr || plaintextBuffer == nullptr || payloadSize < 132 ||
        payload[0] == '#') return false;
    size_t decodedSize = 0;
    if (!nostr::validateBase64Encoding(payload, payloadSize, false, false, decodedSize) || decodedSize < 99 ||
        decodedSize > limits.maxNip44PayloadBytes || plaintextBufferCapacity < decodedSize) return false;
    size_t actualSize = 0;
    const int decodeStatus = mbedtls_base64_decode(
        plaintextBuffer, plaintextBufferCapacity, &actualSize,
        reinterpret_cast<const unsigned char *>(payload), payloadSize);
    if (decodeStatus != 0 || actualSize != decodedSize || plaintextBuffer[0] != 2) {
        wipe(plaintextBuffer, plaintextBufferCapacity);
        return false;
    }
    const size_t paddedSize = decodedSize - 1 - 32 - 32;
    if (paddedSize < 34) {
        wipe(plaintextBuffer, plaintextBufferCapacity);
        return false;
    }
    const uint8_t *nonce = plaintextBuffer + 1;
    uint8_t *ciphertext = plaintextBuffer + 33;
    const uint8_t *receivedMac = ciphertext + paddedSize;
    uint8_t chachaKey[32] = {}, chachaNonce[12] = {}, hmacKey[32] = {}, expectedMac[32] = {};
    bool ok = deriveMessageKeys(conversationKey, nonce, chachaKey, chachaNonce, hmacKey) &&
              hmacParts(hmacKey, sizeof(hmacKey), nonce, 32, ciphertext, paddedSize, expectedMac) &&
              constantTimeEqual(expectedMac, receivedMac, sizeof(expectedMac));
    size_t prefixSize = 0;
    size_t unpaddedSize = 0;
    if (ok) {
        chacha20Xor(ciphertext, paddedSize, chachaKey, chachaNonce);
        ok = paddedPlaintextRange(ciphertext, paddedSize, limits, prefixSize, unpaddedSize);
    }
    if (ok) {
        std::memmove(plaintextBuffer, ciphertext + prefixSize, unpaddedSize);
        wipe(plaintextBuffer + unpaddedSize, decodedSize - unpaddedSize);
        plaintextSize = unpaddedSize;
    } else {
        wipe(plaintextBuffer, plaintextBufferCapacity);
    }
    wipe(chachaKey, sizeof(chachaKey)); wipe(chachaNonce, sizeof(chachaNonce));
    wipe(hmacKey, sizeof(hmacKey)); wipe(expectedMac, sizeof(expectedMac));
    return ok;
}

NostrString encryptRaw(const uint8_t *plaintext, size_t plaintextSize, const uint8_t conversationKey[32],
                       const uint8_t nonce[32], const NostrLimits &limits, NostrString *ownedPlaintext) {
    if (plaintext == nullptr || plaintextSize == 0 || plaintextSize > limits.maxNip44PlaintextBytes ||
        plaintextSize > UINT32_MAX || !validUtf8(plaintext, plaintextSize)) return "";
    const size_t paddedBodySize = calcPaddedLength(plaintextSize);
    const size_t prefixSize = plaintextSize < 65536 ? 2 : 6;
    if (paddedBodySize == 0 || paddedBodySize > SIZE_MAX - prefixSize ||
        prefixSize + paddedBodySize > SIZE_MAX - 65) return "";
    const size_t paddedSize = prefixSize + paddedBodySize;
    const size_t rawSize = 1 + 32 + paddedSize + 32;
    if (rawSize > limits.maxNip44PayloadBytes) return "";
    std::vector<uint8_t> raw;
    try {
        raw.assign(rawSize, 0);
    } catch (...) {
        return "";
    }
    raw[0] = 2;
    std::memcpy(raw.data() + 1, nonce, 32);
    size_t writtenPaddedSize = 0;
    if (!padPlaintextInto(plaintext, plaintextSize, limits, raw.data() + 33, paddedSize, writtenPaddedSize) ||
        writtenPaddedSize != paddedSize) {
        wipe(raw.data(), raw.size());
        return "";
    }
    uint8_t chachaKey[32] = {}, chachaNonce[12] = {}, hmacKey[32] = {}, mac[32] = {};
    if (!deriveMessageKeys(conversationKey, nonce, chachaKey, chachaNonce, hmacKey)) {
        wipe(raw.data(), raw.size());
        return "";
    }
    chacha20Xor(raw.data() + 33, paddedSize, chachaKey, chachaNonce);
    if (!hmacParts(hmacKey, sizeof(hmacKey), raw.data() + 1, 32, raw.data() + 33, paddedSize, mac)) {
        wipe(raw.data(), raw.size());
        wipe(chachaKey, sizeof(chachaKey)); wipe(chachaNonce, sizeof(chachaNonce)); wipe(hmacKey, sizeof(hmacKey));
        return "";
    }
    std::memcpy(raw.data() + 33 + paddedSize, mac, sizeof(mac));
    if (ownedPlaintext != nullptr) *ownedPlaintext = NostrString();
    NostrString result = encodePayload(raw);
    wipe(raw.data(), raw.size());
    wipe(chachaKey, sizeof(chachaKey)); wipe(chachaNonce, sizeof(chachaNonce));
    wipe(hmacKey, sizeof(hmacKey)); wipe(mac, sizeof(mac));
    return result;
}

NostrString decryptRaw(const NostrString &payload, const uint8_t conversationKey[32], const NostrLimits &limits) {
    std::vector<uint8_t> raw;
    if (!decodePayload(payload, limits, raw) || raw.size() < 99) return "";
    const size_t paddedSize = raw.size() - 1 - 32 - 32;
    if (paddedSize < 34) {
        wipe(raw.data(), raw.size());
        return "";
    }
    uint8_t chachaKey[32] = {}, chachaNonce[12] = {}, hmacKey[32] = {}, expectedMac[32] = {};
    const uint8_t *nonce = raw.data() + 1;
    const uint8_t *ciphertext = raw.data() + 33;
    const uint8_t *receivedMac = ciphertext + paddedSize;
    bool ok = deriveMessageKeys(conversationKey, nonce, chachaKey, chachaNonce, hmacKey) &&
              hmacParts(hmacKey, sizeof(hmacKey), nonce, 32, ciphertext, paddedSize, expectedMac) &&
              constantTimeEqual(expectedMac, receivedMac, sizeof(expectedMac));
    NostrString plaintext;
    if (ok) {
        chacha20Xor(raw.data() + 33, paddedSize, chachaKey, chachaNonce);
        ok = unpadPlaintext(raw.data() + 33, paddedSize, limits, plaintext);
    }
    wipe(raw.data(), raw.size());
    wipe(chachaKey, sizeof(chachaKey)); wipe(chachaNonce, sizeof(chachaNonce));
    wipe(hmacKey, sizeof(hmacKey)); wipe(expectedMac, sizeof(expectedMac));
    return ok ? plaintext : NostrString();
}

bool decodeHex32(const NostrString &value, uint8_t output[32]) {
    return nostr::Utils::decodeHexExact(value, output, 32);
}

} // namespace

NostrString nostr::Nip44::conversationKey(const NostrString &privateKeyHex, const NostrString &publicKeyHex) {
    uint8_t privateKey[32] = {}, publicKey[32] = {}, conversation[32] = {};
    const bool ok = decodeHex32(privateKeyHex, privateKey) && decodeHex32(publicKeyHex, publicKey) &&
                    deriveConversationKey(privateKey, publicKey, conversation);
    NostrString result = ok ? nostr::Utils::toHex(conversation, sizeof(conversation)) : NostrString();
    wipe(privateKey, sizeof(privateKey)); wipe(publicKey, sizeof(publicKey)); wipe(conversation, sizeof(conversation));
    return result;
}

NostrString nostr::Nip44::encryptWithConversationKey(NostrString plaintext, const NostrString &conversationKeyHex,
                                                      const NostrString &nonceHex) {
    uint8_t conversation[32] = {}, nonce[32] = {};
    const long plaintextLength = NostrString_length(plaintext);
    const bool ok = decodeHex32(conversationKeyHex, conversation) && decodeHex32(nonceHex, nonce) &&
                    plaintextLength > 0;
    NostrString result = ok ? encryptRaw(reinterpret_cast<const uint8_t *>(plaintext.c_str()),
                                         static_cast<size_t>(plaintextLength), conversation, nonce, NostrLimits(), &plaintext) : NostrString();
    wipe(conversation, sizeof(conversation)); wipe(nonce, sizeof(nonce));
    return result;
}

NostrString nostr::Nip44::decryptWithConversationKey(const NostrString &payload, const NostrString &conversationKeyHex) {
    uint8_t conversation[32] = {};
    const bool ok = decodeHex32(conversationKeyHex, conversation);
    NostrString result = ok ? decryptRaw(payload, conversation, NostrLimits()) : NostrString();
    wipe(conversation, sizeof(conversation));
    return result;
}

bool nostr::Nip44::encryptWithConversationKey(uint8_t *plaintextBuffer, size_t plaintextSize,
                                                size_t plaintextBufferCapacity, const uint8_t conversationKey[32],
                                                const uint8_t nonce[32], char *payloadBuffer, size_t payloadCapacity,
                                                size_t &payloadSize) {
    return encryptBinary(plaintextBuffer, plaintextSize, plaintextBufferCapacity, conversationKey, nonce,
                         payloadBuffer, payloadCapacity, payloadSize);
}

bool nostr::Nip44::decryptWithConversationKey(const char *payload, size_t payloadSize,
                                                const uint8_t conversationKey[32], uint8_t *plaintextBuffer,
                                                size_t plaintextBufferCapacity, size_t &plaintextSize) {
    return decryptBinary(payload, payloadSize, conversationKey, plaintextBuffer, plaintextBufferCapacity, plaintextSize);
}

size_t nostr::Nip44::paddedLength(size_t plaintextLength) {
    return calcPaddedLength(plaintextLength);
}

NostrString nostr::Nip44::encrypt(NostrString &privateKeyHex, NostrString &recipientPubKeyHex, NostrString content) {
    uint8_t privateKey[32] = {}, publicKey[32] = {}, conversation[32] = {}, nonce[32] = {};
    const long plaintextLength = NostrString_length(content);
    const bool ok = decodeHex32(privateKeyHex, privateKey) && decodeHex32(recipientPubKeyHex, publicKey) &&
                    plaintextLength > 0 && static_cast<size_t>(plaintextLength) <= limits.maxNip44PlaintextBytes &&
                    deriveConversationKey(privateKey, publicKey, conversation) && Utils::fillRandom(nonce, sizeof(nonce));
    NostrString result = ok ? encryptRaw(reinterpret_cast<const uint8_t *>(content.c_str()),
                                         static_cast<size_t>(plaintextLength), conversation, nonce, limits, &content) : NostrString();
    wipe(privateKey, sizeof(privateKey)); wipe(publicKey, sizeof(publicKey));
    wipe(conversation, sizeof(conversation)); wipe(nonce, sizeof(nonce));
    return result;
}

NostrString nostr::Nip44::decrypt(NostrString &privateKeyHex, NostrString &senderPubKeyHex, NostrString content) {
    uint8_t privateKey[32] = {}, publicKey[32] = {}, conversation[32] = {};
    const bool ok = decodeHex32(privateKeyHex, privateKey) && decodeHex32(senderPubKeyHex, publicKey) &&
                    deriveConversationKey(privateKey, publicKey, conversation);
    NostrString result = ok ? decryptRaw(content, conversation, limits) : NostrString();
    wipe(privateKey, sizeof(privateKey)); wipe(publicKey, sizeof(publicKey)); wipe(conversation, sizeof(conversation));
    return result;
}
