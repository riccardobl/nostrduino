
#include "NostrEvent.h"
#include <cstring>
using namespace nostr;

const std::vector<NostrEventTag> &NostrEventTags::getTags() const {
    return tags;
}
const std::vector<NostrString> NostrEventTags::emptyTags;

const std::vector<NostrString> &NostrEventTags::getTag(NostrString key) const {
    for (auto &tag : tags) {
        if (NostrString_equals(tag.key, key)) {
            return tag.value;
        }
    }
    return this->emptyTags;
}

unsigned int NostrEventTags::addTag(NostrString key, std::initializer_list<NostrString> values) {
    NostrEventTag tag;
    tag.key = key;
    for (auto &value : values) {
        tag.value.push_back(value);
    }
    tags.push_back(tag);
    return tags.size();
}

unsigned int NostrEventTags::addTag(NostrString key, std::vector<NostrString> values) {
    NostrEventTag tag;
    tag.key = key;
    for (auto &value : values) {
        tag.value.push_back(value);
    }
    tags.push_back(tag);
    return tags.size();
}

void NostrEventTags::removeTag(unsigned int index) {
    tags.erase(tags.begin() + index);
}

void NostrEventTags::removeTags(NostrString key) {
    for (size_t i = 0; i < tags.size(); i++) {
        if (NostrString_equals(tags[i].key, key)) {
            tags.erase(tags.begin() + i);
            i--;
        }
    }
}

void NostrEventTags::clearTags() {
    tags.clear();
}

void NostrEventTags::toJson(JsonArray arr) const {
    for (size_t i = 0; i < tags.size(); i++) {
        JsonArray arr2 = arr.add<JsonArray>();
        arr2.add(tags[i].key);
        for (size_t j = 0; j < tags[i].value.size(); j++) {
            arr2.add(tags[i].value[j]);
        }
    }
}


SignedNostrEvent UnsignedNostrEvent::sign(NostrString privateKeyHex) {
    Utils::log("Signing event with private key");
    
    NostrString pubKeyHex = Utils::getPublicKey(privateKeyHex);
    if (NostrString_length(pubKeyHex) != 64) return SignedNostrEvent();

    JsonDocument doc;
    JsonArray data = doc["data"].to<JsonArray>();
    data.add(0);
    data.add(pubKeyHex);
    data.add(this->created_at);
    data.add(this->kind);
    JsonArray tags = data.add<JsonArray>();
    this->getTags()->toJson(tags);
    data.add(this->content);

    NostrString message;
    Utils::jsonStringify(data, &message);
    doc.clear();
    byte hash[64] = {0};
    int hashLen = 0;
    hashLen = Utils::sha256(message, hash);
    NostrString msgHash = Utils::toHex(hash, hashLen);

    Utils::log("Message hash: " + msgHash);

    byte messageBytes[NOSTR_DIGEST_SIZE];
    if (!Utils::decodeHexExact(msgHash, messageBytes, sizeof(messageBytes))) return SignedNostrEvent();
    
    NostrString signatureHex = Utils::sign(privateKeyHex, messageBytes, NOSTR_DIGEST_SIZE);
    if (NostrString_length(signatureHex) != 128) return SignedNostrEvent();
    
    SignedNostrEvent signedEvent(msgHash, pubKeyHex, this->created_at, this->kind, this->tags, this->content, signatureHex);
    Utils::log("Signature hash " + signatureHex);

    return signedEvent;
}

bool SignedNostrEvent::verify() const {
    if (!this->valid) return false;
    if (this->verifyState != -1) {
        return this->verifyState == 1;
    }
    Utils::log("Verifying event id");
    JsonDocument doc;
    JsonArray data = doc["data"].to<JsonArray>();
    data.add(0);
    data.add(this->pubkey);
    data.add(this->created_at);
    data.add(this->kind);
    JsonArray tags = data.add<JsonArray>();
    this->getTags()->toJson(tags);
    data.add(this->content);
    NostrString message;
    Utils::jsonStringify(data, &message);
    doc.clear();
    byte hash[64] = {0};
    int hashLen = 0;
    hashLen = Utils::sha256(message, hash);
    NostrString msgHash = Utils::toHex(hash, hashLen);
    if(!NostrString_equals(msgHash, this->id)){
        Utils::log("Event id does not match");
        this->verifyState = 0;
        return false;
    }
    Utils::log("Verifying event signature");
    byte messageBytes[32];
    if (!Utils::decodeHexExact(this->id, messageBytes, sizeof(messageBytes))) {
        this->verifyState = 0;
        return false;
    }
    Utils::log("Message hash: " + this->id);
   
    Utils::log("Pubkey: " + this->pubkey);
    Utils::log("Signature: " + this->signature);
    bool verified = Utils::verify(this->pubkey, messageBytes, this->signature);
    this->verifyState = verified ? 1 : 0;
    return verified;
}

void SignedNostrEvent::toJson(JsonObject doc) const {
    doc["id"] = this->id;
    doc["pubkey"] = this->pubkey;
    doc["created_at"] = this->created_at;
    doc["kind"] = this->kind;
    JsonArray tags = doc["tags"].to<JsonArray>();
    this->tags.toJson(tags);
    doc["content"] = this->content;
    doc["sig"] = this->signature;
}

void SignedNostrEvent::toSendableEvent(JsonArray doc) const {
    doc.add("EVENT");
    JsonObject obj = doc.add<JsonObject>();
    this->toJson(obj);
}

bool SignedNostrEvent::parseRelayFrame(JsonArrayConst arr, SignedNostrEvent &out, const NostrLimits &limits) {
    out = SignedNostrEvent();
    SignedNostrEvent parsed;
    try {
        if (arr.size() != 3 || !arr[0].is<const char *>() ||
            std::strcmp(arr[0].as<const char *>(), "EVENT") != 0 ||
            !arr[1].is<const char *>() || !arr[2].is<JsonObjectConst>()) return false;

        JsonObjectConst obj = arr[2].as<JsonObjectConst>();
        JsonVariantConst id = obj["id"];
        JsonVariantConst pubkey = obj["pubkey"];
        JsonVariantConst content = obj["content"];
        JsonVariantConst signature = obj["sig"];
        JsonVariantConst createdAt = obj["created_at"];
        JsonVariantConst kind = obj["kind"];
        JsonVariantConst rawTags = obj["tags"];
        if (!id.is<const char *>() || !pubkey.is<const char *>() ||
            !content.is<const char *>() || !signature.is<const char *>() ||
            !createdAt.is<unsigned long>() || !kind.is<unsigned int>() ||
            !rawTags.is<JsonArrayConst>()) return false;
        JsonArrayConst tags = rawTags.as<JsonArrayConst>();
        if (std::strlen(arr[1].as<const char *>()) > limits.maxSubscriptionIdBytes ||
            std::strlen(content.as<const char *>()) > limits.maxEventContentBytes ||
            std::strlen(id.as<const char *>()) != 64 || std::strlen(pubkey.as<const char *>()) != 64 ||
            std::strlen(signature.as<const char *>()) != 128 || tags.size() > limits.maxTagCount) return false;

        uint8_t fieldBytes[64];
        if (!Utils::decodeHexExact(id.as<const char *>(), fieldBytes, 32) ||
            !Utils::decodeHexExact(pubkey.as<const char *>(), fieldBytes, 32) ||
            !Utils::decodeHexExact(signature.as<const char *>(), fieldBytes, 64)) return false;

        NostrEventTags parsedTags;
        for (size_t i = 0; i < tags.size(); ++i) {
            if (!tags[i].is<JsonArrayConst>()) return false;
            JsonArrayConst tag = tags[i].as<JsonArrayConst>();
            if (tag.size() == 0 || tag.size() > limits.maxTagElements || !tag[0].is<const char *>()) return false;
            if (std::strlen(tag[0].as<const char *>()) > limits.maxTagElementBytes) return false;
            std::vector<NostrString> values;
            values.reserve(tag.size() - 1);
            for (size_t j = 1; j < tag.size(); ++j) {
                if (!tag[j].is<const char *>()) return false;
                if (std::strlen(tag[j].as<const char *>()) > limits.maxTagElementBytes) return false;
                values.push_back(tag[j].as<const char *>());
            }
            parsedTags.addTag(tag[0].as<const char *>(), values);
        }

        parsed.id = id.as<const char *>();
        parsed.pubkey = pubkey.as<const char *>();
        parsed.subid = arr[1].as<const char *>();
        parsed.created_at = createdAt.as<unsigned long>();
        parsed.kind = kind.as<unsigned int>();
        parsed.tags = parsedTags;
        parsed.content = content.as<const char *>();
        parsed.signature = signature.as<const char *>();
        parsed.valid = true;
        out = parsed;
        return true;
    } catch (...) {
        return false;
    }
}
