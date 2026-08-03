#ifndef NOSTR_UTILS_H
#define NOSTR_UTILS_H
#include <Arduino.h>

#include "ArduinoJson.h"
#include "NostrEncoding.h"
#include "NostrString.h"
#include <functional>
#define NOSTR_DIGEST_SIZE 32
#define BASE64_STANDARD 0
#define BASE64_NOPADDING 1
#define BASE64_URLSAFE 2

namespace nostr {

class Utils {
  public:
    static std::function<void(const NostrString &)> logger;
    static unsigned long long subs;
    static std::function<bool(uint8_t *, size_t)> randomBytesProvider;
    static std::function<unsigned long long()> timeSecondsProvider;

    static void setUnixTimeSecondsProvider(std::function<unsigned long long()> timeSecondsProvider);
    static void setRandomBytesProvider(std::function<bool(uint8_t *, size_t)> randomBytesProvider);
    static void setLogger(std::function<void(const NostrString &)> logger);
    static void log(const NostrString &message);
    static long long unixTimeSeconds();
    static NostrString getNewSubscriptionId();
    static long int randomInt(long int min, long int max);
    static bool fillRandom(uint8_t *output, size_t outputSize);
    static NostrString getPublicKey(NostrString privKeyHex);
    static bool jsonParse(const NostrString *json, JsonDocument *doc);
    static void jsonStringify(JsonVariantConst source, NostrString *json);
    static NostrString toHex(const uint8_t *array, size_t arraySize);
    static bool decodeHexExact(const NostrString &input, uint8_t *output, size_t outputCapacity, size_t expectedBytes);
    static bool decodeHexExact(const NostrString &input, uint8_t *output, size_t expectedBytes);
    static NostrString hexToBase64(NostrString hex);
    static size_t fromHex(NostrString hex, uint8_t *output, size_t outputSize);
    static size_t fromBase64(const char *encoded, size_t encodedSize, uint8_t *output, size_t outputSize, uint8_t flags);
    static bool ecdh(NostrString privKeyHex, NostrString pubKeyHex, uint8_t *out);
    static size_t toBase64(const uint8_t *array, size_t arraySize, char *output, size_t outputSize, uint8_t flags) ;
    static size_t sha256(NostrString message, uint8_t *hash);
    static size_t sha256(const uint8_t *message, size_t messageSize, uint8_t *hash);
    static NostrString sign(NostrString privateKeyHex, uint8_t *message, size_t messageSize);
    static NostrString sign(NostrString privateKeyHex, uint8_t *message, size_t messageSize, const uint8_t auxiliaryRandomness[32]);
    static bool verify(NostrString pubKeyHex, uint8_t *messageHash,  NostrString signatureHex);
    static NostrString base64ToHex(NostrString base64ToHex);
    static void init();
    static void close();
};

}


#endif
