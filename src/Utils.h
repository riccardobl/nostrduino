#ifndef NOSTR_UTILS_H
#define NOSTR_UTILS_H
#include <Arduino.h>

#include "ArduinoJson.h"
#include "NostrString.h"
#include <functional>
#define NOSTR_DIGEST_SIZE 32
// #define BASE64_STANDARD 0
// #define BASE64_NOPADDING 1
// #define BASE64_URLSAFE 2

namespace nostr {

class Utils {
  public:
    static std::function<void(const NostrString &)> logger;
    static unsigned long long subs;
    static std::function<long int(long int min, long int max)> realRandom;
    static std::function<unsigned long long()> timeSecondsProvider;

    static void setUnixTimeSecondsProvider(std::function<unsigned long long()> timeSecondsProvider);
    static void setRealRandom(std::function<long int(long int min, long int max)> realRandom);
    static void setLogger(std::function<void(const NostrString &)> logger);
    static void log(const NostrString &message);
    static long long unixTimeSeconds();
    static NostrString getNewSubscriptionId();
    static long int randomInt(long int min, long int max);
    static NostrString getPublicKey(NostrString privKeyHex);
    static void jsonParse(const NostrString *json, JsonDocument *doc);
    static void jsonStringify(JsonVariantConst source, NostrString *json);
    static NostrString toHex(const uint8_t *array, size_t arraySize);
    static NostrString hexToBase64(NostrString hex);
    static size_t fromHex(NostrString hex, uint8_t *output, size_t outputSize);
    static size_t fromBase64(const char *encoded, size_t encodedSize, uint8_t *output, size_t outputSize, uint8_t flags);
    static void ecdh(NostrString privKeyHex, NostrString pubKeyHex, uint8_t *out);
    static size_t toBase64(const uint8_t *array, size_t arraySize, char *output, size_t outputSize, uint8_t flags) ;
    static size_t sha256(NostrString message, uint8_t *hash);
    static NostrString sign(NostrString privateKeyHex, uint8_t *message, size_t messageSize);
    static bool verify(NostrString pubKeyHex, uint8_t *messageHash,  NostrString signatureHex);
    static NostrString base64ToHex(NostrString base64ToHex);
    static void init();
    static void close();
    private:
    static void *secp256k1_ctx;
};
}


#endif