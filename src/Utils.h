#ifndef NOSTR_UTILS_H
#define NOSTR_UTILS_H

#include <stdlib.h>
#include <functional>
#include <string>
#include "ArduinoJson.h"
#include "Bitcoin.h"
#include "NostrString.h"
#define NOSTR_DIGEST_SIZE 32

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
};
}  

#endif