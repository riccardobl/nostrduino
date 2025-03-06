#include "NostrUtils.h"

#include "Hash.h" // depends on uBitcoin: https://github.com/micro-bitcoin/uBitcoin
#include "Bitcoin.h" // depends on uBitcoin: https://github.com/micro-bitcoin/uBitcoin

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

void Utils::jsonParse(const NostrString *json, JsonDocument *doc) {
    DeserializationError error = deserializeJson(*doc, *json);
    if (error) {
        throw std::runtime_error(error.c_str());
    }
}

void Utils::jsonStringify(JsonVariantConst source, NostrString *json) {
    serializeJson(source, *json);
}

NostrString Utils::toHex(const uint8_t *array, size_t arraySize) {
    return ::toHex(array, arraySize);    
}

NostrString Utils::hexToBase64(NostrString hex) {
    return ::hexToBase64(hex);
}

NostrString Utils::base64ToHex(NostrString base64) {
    return ::base64ToHex(base64);
}

size_t Utils::fromHex(NostrString hex, uint8_t *output, size_t outputSize) {
    return ::fromHex(hex, output, outputSize);
}

size_t Utils::fromBase64(const char *encoded, size_t encodedSize, uint8_t *output, size_t outputSize, uint8_t flags) {
    return ::fromBase64(encoded, encodedSize, output, outputSize, flags);
}


size_t Utils::toBase64(const uint8_t *array, size_t arraySize, char *output, size_t outputSize, uint8_t flags) {
    return ::toBase64(array, arraySize, output, outputSize, flags);
}


size_t Utils::sha256(NostrString message, uint8_t *hash) {
    return ::sha256(message, hash);
}

NostrString Utils::getPublicKey(NostrString privKeyHex) {
    byte privateKeyBytes[NOSTR_DIGEST_SIZE];
    fromHex(privKeyHex, privateKeyBytes, NOSTR_DIGEST_SIZE);
    PrivateKey privateKey(privateKeyBytes);

    PublicKey pubKey = privateKey.publicKey();
    NostrString pubKeyHex = pubKey.toString();
    pubKeyHex = NostrString_substring(pubKeyHex, 2);
    return pubKeyHex;   
}


void Utils::ecdh(NostrString privateKeyHex, NostrString pubKeyHex, uint8_t *out) {

    int byteSize = 32;
    byte privateKeyBytes[byteSize];
    fromHex(privateKeyHex, privateKeyBytes, byteSize);
    PrivateKey privateKey(privateKeyBytes);

    uint8_t pubkeyBytes[64];
    fromHex("02" + NostrString(pubKeyHex), pubkeyBytes, 64);
    PublicKey otherDhPublicKey(pubkeyBytes);
    privateKey.ecdh(otherDhPublicKey, out, false);
}

NostrString Utils::sign(NostrString privateKeyHex, uint8_t *messageBytes, size_t messageSize) {
    if(messageSize != 32){
        throw std::runtime_error("Message size must be 32 bytes");
    }
    uint8_t privateKeyBytes[32];
    fromHex(privateKeyHex, privateKeyBytes, 32);
    PrivateKey privateKey(privateKeyBytes);
    SchnorrSignature signature = privateKey.schnorr_sign(messageBytes);
    NostrString signatureHex = signature.toString();
    return signatureHex;
}

bool Utils::verify(NostrString pubKeyHex, uint8_t *messageHash, NostrString signatureHex){
    byte pubkeyBytes[64];
    fromHex("02" + pubKeyHex, pubkeyBytes, 64);
    PublicKey pub(pubkeyBytes);
    byte signatureBytes[64];
    fromHex(signatureHex, signatureBytes, 64);
    SchnorrSignature signature(signatureBytes);
    return pub.schnorr_verify(signature, messageHash);
}


void Utils::init() {
  }

void Utils::close() {
}