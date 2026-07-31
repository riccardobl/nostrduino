#include "NostrUtils.h"
#include "NostrEncoding.h"
#include "crypto/UBitcoinBackend.h"
#include <Arduino.h>
#include <mbedtls/base64.h>
#include <mbedtls/sha256.h>
#include <cstring>
#include <limits>
#include <vector>

using namespace nostr;

std::function<void(const NostrString &)> Utils::logger = nullptr;
unsigned long long Utils::subs = 0;
std::function<bool(uint8_t *, size_t)> Utils::randomBytesProvider = nullptr;
std::function<unsigned long long()> Utils::timeSecondsProvider = nullptr;

namespace {
void wipe(void *memory, size_t size) {
    if (memory == nullptr) return;
    volatile uint8_t *p = static_cast<volatile uint8_t *>(memory);
    while (size-- > 0) *p++ = 0;
}

struct WipingBytes {
    std::vector<uint8_t> value;
    ~WipingBytes() {
        if (!value.empty()) wipe(value.data(), value.size());
    }
};

NostrString hexEncode(const uint8_t *array, size_t arraySize) {
    if (array == nullptr && arraySize != 0) return "";
    if (arraySize > (SIZE_MAX / 2)) return "";
    static const char hex[] = "0123456789abcdef";
    NostrString result;
    result.reserve(arraySize * 2);
    for (size_t i = 0; i < arraySize; ++i) {
        result += hex[array[i] >> 4];
        result += hex[array[i] & 0x0f];
    }
    return result;
}

bool base64DecodeStrict(const char *encoded, size_t encodedSize, uint8_t *output, size_t outputSize, uint8_t flags, size_t &decodedSize) {
    decodedSize = 0;
    if (output != nullptr) wipe(output, outputSize);
    if (encoded == nullptr || output == nullptr) return false;
    const bool noPadding = (flags & BASE64_NOPADDING) != 0;
    const bool urlSafe = (flags & BASE64_URLSAFE) != 0;
    size_t expectedSize = 0;
    if (!validateBase64Encoding(encoded, encodedSize, noPadding, urlSafe, expectedSize) ||
        expectedSize > outputSize) return false;

    try {
        NostrString normalized;
        const size_t extraPadding = noPadding ? (4 - encodedSize % 4) % 4 : 0;
        if (encodedSize > SIZE_MAX - extraPadding) return false;
        normalized.reserve(encodedSize + extraPadding);
        for (size_t i = 0; i < encodedSize; ++i) {
            const char c = encoded[i];
            normalized += c == '-' ? '+' : (c == '_' ? '/' : c);
        }
        if (noPadding) {
            while (normalized.length() % 4 != 0) normalized += '=';
        }
        size_t result = 0;
        if (mbedtls_base64_decode(output, outputSize, &result,
                                  reinterpret_cast<const unsigned char *>(normalized.c_str()), normalized.length()) != 0 ||
            result != expectedSize) {
            wipe(output, outputSize);
            return false;
        }
        decodedSize = result;
        return true;
    } catch (...) {
        wipe(output, outputSize);
        return false;
    }
}
}

void Utils::setUnixTimeSecondsProvider(std::function<unsigned long long()> provider) {
    Utils::timeSecondsProvider = provider;
}

void Utils::setRealRandom(std::function<long int(long int min, long int max)> provider) {
    if (!provider) {
        setRandomBytesProvider(nullptr);
        return;
    }
    setRandomBytesProvider([provider](uint8_t *output, size_t outputSize) {
        if (output == nullptr && outputSize != 0) return false;
        for (size_t i = 0; i < outputSize; ++i) {
            output[i] = static_cast<uint8_t>(provider(0, 255));
        }
        return true;
    });
}

void Utils::setRandomBytesProvider(std::function<bool(uint8_t *, size_t)> provider) {
    Utils::randomBytesProvider = provider;
}

void Utils::setLogger(std::function<void(const NostrString &)> provider) {
    Utils::logger = provider;
}

void Utils::log(const NostrString &message) {
    if (Utils::logger) Utils::logger(message);
}

long long Utils::unixTimeSeconds() {
    if (!timeSecondsProvider) {
        Utils::log("No unix time provider set");
        return 0;
    }
    return timeSecondsProvider();
}

NostrString Utils::getNewSubscriptionId() {
    NostrString chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    NostrString result = NostrString_intToString(subs++);
    const size_t maxLength = 64;
    for (size_t i = NostrString_length(result); i < maxLength; ++i) {
        uint8_t sample = 0;
        do {
            if (!fillRandom(&sample, sizeof(sample))) return "";
        } while (sample >= 248);
        result += chars[sample % 62];
    }
    return result;
}

long int Utils::randomInt(long int min, long int max) {
    if (min > max) return 0;

    // Work in the unsigned representation so intervals crossing zero and the
    // complete signed range are handled without signed arithmetic overflow.
    const uint64_t low = static_cast<uint64_t>(min);
    const uint64_t high = static_cast<uint64_t>(max);
    const uint64_t span = high - low + 1;
    const uint64_t threshold = span == 0 ? 0 : (static_cast<uint64_t>(0) - span) % span;
    uint64_t sample = 0;
    do {
        if (!fillRandom(reinterpret_cast<uint8_t *>(&sample), sizeof(sample))) return 0;
    } while (sample < threshold);

    const uint64_t offset = span == 0 ? sample : sample % span;
    return static_cast<long int>(low + offset);
}

bool Utils::fillRandom(uint8_t *output, size_t outputSize) {
    if (output == nullptr && outputSize != 0) return false;
    if (outputSize == 0) return true;
    if (!randomBytesProvider || !randomBytesProvider(output, outputSize)) {
        if (output != nullptr) wipe(output, outputSize);
        Utils::log("No random byte provider set");
        return false;
    }
    return true;
}

bool Utils::jsonParse(const NostrString *json, JsonDocument *doc) {
    if (json == nullptr || doc == nullptr) return false;
    DeserializationError error = deserializeJson(*doc, *json);
    if (error) {
        doc->clear();
        return false;
    }
    return true;
}

void Utils::jsonStringify(JsonVariantConst source, NostrString *json) {
    if (json != nullptr) serializeJson(source, *json);
}

NostrString Utils::toHex(const uint8_t *array, size_t arraySize) {
    return hexEncode(array, arraySize);
}

bool Utils::decodeHexExact(const NostrString &input, uint8_t *output, size_t outputCapacity, size_t expectedBytes) {
    return nostr::decodeHexExact(input, output, outputCapacity, expectedBytes);
}

bool Utils::decodeHexExact(const NostrString &input, uint8_t *output, size_t expectedBytes) {
    return nostr::decodeHexExact(input, output, expectedBytes);
}

NostrString Utils::hexToBase64(NostrString hex) {
    const long hexLength = NostrString_length(hex);
    if (hexLength < 0 || (static_cast<size_t>(hexLength) & 1u) != 0) return "";
    const size_t byteLength = static_cast<size_t>(hexLength) / 2;
    try {
        WipingBytes bytes;
        bytes.value.resize(byteLength);
        if (!nostr::decodeHexExact(hex, bytes.value.data(), bytes.value.size())) return "";
        if (byteLength > SIZE_MAX - 2) return "";
        const size_t groups = (byteLength + 2) / 3;
        if (groups > (SIZE_MAX - 1) / 4) return "";
        const size_t encodedCapacity = groups * 4;
        if (encodedCapacity == 0 && byteLength != 0) return "";
        std::vector<char> encoded(encodedCapacity + 1);
        const size_t encodedSize = Utils::toBase64(bytes.value.data(), bytes.value.size(), encoded.data(), encoded.size(), BASE64_STANDARD);
        if (encodedSize == 0 && byteLength != 0) return "";
        NostrString result;
        result.reserve(encodedSize);
        for (size_t i = 0; i < encodedSize; ++i) result += encoded[i];
        return result;
    } catch (...) {
        return "";
    }
}

NostrString Utils::base64ToHex(NostrString base64) {
    const long encodedLength = NostrString_length(base64);
    if (encodedLength <= 0) return "";
    try {
        const size_t encodedSize = static_cast<size_t>(encodedLength);
        if (encodedSize == 0 || encodedSize % 4 != 0 || encodedSize / 4 > SIZE_MAX / 3) return "";
        const size_t capacity = (encodedSize / 4) * 3;
        WipingBytes decoded;
        decoded.value.resize(capacity);
        const size_t decodedSize = Utils::fromBase64(base64.c_str(), encodedSize,
                                                     decoded.value.data(), decoded.value.size(), BASE64_STANDARD);
        return decodedSize == 0 ? "" : hexEncode(decoded.value.data(), decodedSize);
    } catch (...) {
        return "";
    }
}

size_t Utils::fromHex(NostrString hex, uint8_t *output, size_t outputSize) {
    return decodeHexExact(hex, output, outputSize, outputSize) ? outputSize : 0;
}

size_t Utils::fromBase64(const char *encoded, size_t encodedSize, uint8_t *output, size_t outputSize, uint8_t flags) {
    size_t decodedSize = 0;
    return base64DecodeStrict(encoded, encodedSize, output, outputSize, flags, decodedSize) ? decodedSize : 0;
}

size_t Utils::toBase64(const uint8_t *array, size_t arraySize, char *output, size_t outputSize, uint8_t flags) {
    if (output != nullptr) wipe(output, outputSize);
    if (array == nullptr || output == nullptr || arraySize > ((SIZE_MAX - 2) / 4) * 3) return 0;
    const size_t standardSize = ((arraySize + 2) / 3) * 4;
    const bool noPadding = (flags & BASE64_NOPADDING) != 0;
    const size_t padding = (arraySize % 3 == 0) ? 0 : 3 - (arraySize % 3);
    const size_t finalSize = standardSize - (noPadding ? padding : 0);
    if (outputSize < finalSize) return 0;
    try {
        std::vector<unsigned char> encoded(standardSize + 1);
        size_t encodedSize = 0;
        if (mbedtls_base64_encode(encoded.data(), encoded.size(), &encodedSize, array, arraySize) != 0 || encodedSize != standardSize) {
            wipe(encoded.data(), encoded.size());
            return 0;
        }
        size_t writeIndex = 0;
        for (size_t i = 0; i < encodedSize; ++i) {
            if (noPadding && encoded[i] == '=') continue;
            char c = static_cast<char>(encoded[i]);
            if ((flags & BASE64_URLSAFE) != 0) c = c == '+' ? '-' : (c == '/' ? '_' : c);
            output[writeIndex++] = c;
        }
        wipe(encoded.data(), encoded.size());
        return writeIndex;
    } catch (...) {
        return 0;
    }
}

size_t Utils::sha256(NostrString message, uint8_t *hash) {
    if (hash == nullptr) return 0;
    mbedtls_sha256_context context;
    mbedtls_sha256_init(&context);
    const bool ok = mbedtls_sha256_starts_ret(&context, 0) == 0 &&
                    mbedtls_sha256_update_ret(&context, reinterpret_cast<const unsigned char *>(message.c_str()), message.length()) == 0 &&
                    mbedtls_sha256_finish_ret(&context, hash) == 0;
    mbedtls_sha256_free(&context);
    if (!ok) wipe(hash, NOSTR_DIGEST_SIZE);
    return ok ? NOSTR_DIGEST_SIZE : 0;
}

NostrString Utils::getPublicKey(NostrString privateKeyHex) {
    uint8_t privateKey[32] = {};
    uint8_t publicKey[32] = {};
    if (!decodeHexExact(privateKeyHex, privateKey, sizeof(privateKey)) ||
        !crypto::UBitcoinBackend::publicKey(privateKey, publicKey)) {
        wipe(privateKey, sizeof(privateKey));
        wipe(publicKey, sizeof(publicKey));
        return "";
    }
    NostrString result = hexEncode(publicKey, sizeof(publicKey));
    wipe(privateKey, sizeof(privateKey));
    wipe(publicKey, sizeof(publicKey));
    return result;
}

bool Utils::ecdh(NostrString privateKeyHex, NostrString pubKeyHex, uint8_t *out) {
    if (out == nullptr) return false;
    wipe(out, 32);
    uint8_t privateKey[32] = {};
    uint8_t publicKey[32] = {};
    const bool ok = decodeHexExact(privateKeyHex, privateKey, sizeof(privateKey)) &&
                    decodeHexExact(pubKeyHex, publicKey, sizeof(publicKey)) &&
                    crypto::UBitcoinBackend::ecdhRawX(privateKey, publicKey, out);
    wipe(privateKey, sizeof(privateKey));
    wipe(publicKey, sizeof(publicKey));
    return ok;
}

NostrString Utils::sign(NostrString privateKeyHex, uint8_t *message, size_t messageSize) {
    if (message == nullptr || messageSize != 32) return "";
    uint8_t auxiliaryRandomness[32] = {};
    if (!fillRandom(auxiliaryRandomness, sizeof(auxiliaryRandomness))) return "";
    NostrString result = sign(privateKeyHex, message, messageSize, auxiliaryRandomness);
    wipe(auxiliaryRandomness, sizeof(auxiliaryRandomness));
    return result;
}

NostrString Utils::sign(NostrString privateKeyHex, uint8_t *message, size_t messageSize, const uint8_t auxiliaryRandomness[32]) {
    if (message == nullptr || messageSize != 32 || auxiliaryRandomness == nullptr) return "";
    uint8_t privateKey[32] = {};
    uint8_t signature[64] = {};
    const bool ok = decodeHexExact(privateKeyHex, privateKey, sizeof(privateKey)) &&
                    crypto::UBitcoinBackend::sign(privateKey, message, messageSize, auxiliaryRandomness, signature);
    NostrString result = ok ? hexEncode(signature, sizeof(signature)) : "";
    wipe(privateKey, sizeof(privateKey));
    wipe(signature, sizeof(signature));
    return result;
}

bool Utils::verify(NostrString pubKeyHex, uint8_t *messageHash, NostrString signatureHex) {
    uint8_t publicKey[32] = {};
    uint8_t signature[64] = {};
    const bool ok = messageHash != nullptr && decodeHexExact(pubKeyHex, publicKey, sizeof(publicKey)) &&
                    decodeHexExact(signatureHex, signature, sizeof(signature)) &&
                    crypto::UBitcoinBackend::verify(publicKey, messageHash, 32, signature);
    wipe(publicKey, sizeof(publicKey));
    wipe(signature, sizeof(signature));
    return ok;
}

void Utils::init() {
    crypto::UBitcoinBackend::initialize([](uint8_t *output, size_t outputSize) {
        return Utils::fillRandom(output, outputSize);
    });
}

void Utils::close() {
    crypto::UBitcoinBackend::shutdown();
}
