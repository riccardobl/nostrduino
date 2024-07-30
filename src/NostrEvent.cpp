
#include "NostrEvent.h"
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
    Utils::fromHex(msgHash, messageBytes, NOSTR_DIGEST_SIZE);
    
    NostrString signatureHex = Utils::sign(privateKeyHex, messageBytes, NOSTR_DIGEST_SIZE);
    
    SignedNostrEvent signedEvent(msgHash, pubKeyHex, this->created_at, this->kind, this->tags, this->content, signatureHex);
    Utils::log("Signature hash " + signatureHex);

    return signedEvent;
}

bool SignedNostrEvent::verify() const {
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
        return false;
    }
    Utils::log("Verifying event signature");
    byte messageBytes[32];
    Utils::fromHex(this->id, messageBytes, 32);
    Utils::log("Message hash: " + this->id);
   
    Utils::log("Pubkey: " + this->pubkey);
    Utils::log("Signature: " + this->signature);
    return Utils::verify(this->pubkey, messageBytes, this->signature);
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

SignedNostrEvent::SignedNostrEvent(JsonArray arr) {
    if (arr[0] != "EVENT") {
        throw std::runtime_error("Invalid event type");
    }
    JsonObject obj;
    if (arr.size() > 2) {
        obj = arr[2];
        this->subid = arr[1].as<NostrString>();
    } else {
        obj = arr[1];
    }
    this->id = obj["id"].as<NostrString>();
    this->pubkey = obj["pubkey"].as<NostrString>();
    this->created_at = obj["created_at"].as<unsigned long long>();
    this->kind = obj["kind"].as<unsigned int>();
    NostrEventTags tt = NostrEventTags();
    JsonArray tags = obj["tags"].as<JsonArray>();
    for (size_t i = 0; i < tags.size(); i++) {
        JsonArray tag = tags[i];
        std::vector<String> value;
        for (size_t j = 1; j < tag.size(); j++) {
            value.push_back(tag[j].as<NostrString>());
        }
        tt.addTag(tag[0].as<NostrString>(), value);
    }
    this->tags = tt;
    this->content = obj["content"].as<NostrString>();
    this->signature = obj["sig"].as<NostrString>();
}
