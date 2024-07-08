#include "Utils.h"

using namespace nostr;

std::function<void(const NostrString &)> Utils::logger = nullptr;
unsigned long long Utils::subs = 0;
std::function<long int(long int min, long int max)> Utils::realRandom = nullptr;
std::function<unsigned long long()> Utils::timeSecondsProvider = nullptr;
void Utils::setUnixTimeSecondsProvider(std::function<unsigned long long()> timeSecondsProvider) {
    Utils::timeSecondsProvider = timeSecondsProvider;
}
void Utils::setRealRandom(std::function<long int(long int min, long int max)> realRandom) {
    Utils::realRandom = realRandom;
}
void Utils::setLogger(std::function<void(const NostrString &)> logger) {
    Utils::logger = logger;
}
void Utils::log(const NostrString &message) {
    if (Utils::logger) {
        Utils::logger(message);
    }
}
long long Utils::unixTimeSeconds() {
    if (!timeSecondsProvider) {
        Utils::log("No unix time provider set");
        return 0;
    }
    return timeSecondsProvider();
}

NostrString Utils::getNewSubscriptionId() {
    NostrString chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ012345"
                        "6789";
    NostrString result = "";
    result += NostrString_intToString(subs++);
    int maxLength = 64;
    for (int i = NostrString_length(result); i < maxLength; i++) {
        int randomIndex = Utils::randomInt(0, NostrString_length(chars) - 1);
        result += chars[randomIndex];
    }
    return result;
}

long int Utils::randomInt(long int min, long int max) {
    if (!realRandom) {
        Utils::log("No random function set");
        return 0;
    }
    return realRandom(min, max);
}

NostrString Utils::getPublicKey(NostrString privKeyHex) {
    byte privateKeyBytes[NOSTR_DIGEST_SIZE];
    NostrString_hexToBytes(privKeyHex, privateKeyBytes, NOSTR_DIGEST_SIZE);
    PrivateKey privateKey(privateKeyBytes);

    byte pubKeyBytes[NOSTR_DIGEST_SIZE];
    PublicKey pubKey = privateKey.publicKey();
    NostrString pubKeyHex = pubKey.toString();
    pubKeyHex = NostrString_substring(pubKeyHex, 2);
    return pubKeyHex;
}

void Utils::jsonParse(const NostrString *json, JsonDocument *doc) {
    DeserializationError error = deserializeJson(*doc, *json);
    if (error) {
        throw std::runtime_error(error.c_str());
    }
}

void Utils::jsonStringify(JsonVariantConst source, NostrString *json) {
    serializeJson(source, *json);
}