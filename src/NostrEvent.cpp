
#include "NostrEvent.h"
 



using namespace nostr;


const std::vector<NostrEventTag> &NostrEventTags::getTags() const
{
    return tags;
}
const std::vector<NostrString> NostrEventTags::emptyTags;

const std::vector<NostrString> &NostrEventTags::getTag(NostrString key) const {
    for (auto &tag : tags)
    {
        if (NostrString_equals(tag.key , key)) {
            return tag.value;
        }
    }
    return this->emptyTags;
}

unsigned int NostrEventTags::addTag(NostrString key,
                                    std::initializer_list<NostrString> values) {
    NostrEventTag tag;
    tag.key = key;
    for (auto &value : values)
    {
        tag.value.push_back(value);
    }
    tags.push_back(tag);
    return tags.size();
}

unsigned int NostrEventTags::addTag(NostrString key,
                                    std::vector<NostrString> values) {
    NostrEventTag tag;
    tag.key = key;
    for (auto &value : values)
    {
        tag.value.push_back(value);
    }
    tags.push_back(tag);
    return tags.size();
}

void NostrEventTags::removeTag(unsigned int index)
{
    tags.erase(tags.begin() + index);
}

void NostrEventTags::removeTags(NostrString key) {
    for (int i = 0; i < tags.size(); i++)
    {
        if (NostrString_equals(tags[i].key ,key))
        {
            tags.erase(tags.begin() + i);
            i--;
        }
    }
}

void NostrEventTags::clearTags()
{
    tags.clear();
}

void NostrEventTags::toJson(JsonArray arr) const
{
    for (int i = 0; i < tags.size(); i++) {
        JsonArray arr2 = arr.add<JsonArray>();
        arr2.add(tags[i].key);
        for (int j = 0; j < tags[i].value.size(); j++) {
            arr2.add(tags[i].value[j]);
        }
    }
}

SignedNostrEvent UnsignedNostrEvent::sign(NostrString privateKeyHex) {
    Utils::log("Signing event with private key");
    byte privateKeyBytes[NOSTR_DIGEST_SIZE];
    NostrString_hexToBytes(privateKeyHex, privateKeyBytes, NOSTR_DIGEST_SIZE);
    PrivateKey privateKey(privateKeyBytes);

    byte pubKeyBytes[NOSTR_DIGEST_SIZE];
    PublicKey pubKey = privateKey.publicKey();
    NostrString pubKeyHex = pubKey.toString();
    pubKeyHex = NostrString_substring(pubKeyHex, 2);

    Utils::log("PubKey " + pubKeyHex);

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
    serializeJson(doc["data"], message);
    doc.clear();

    Utils::log("Compute signature of: "+message);

    // sha256 of message converted to hex, assign to msghash
    byte hash[64] = {0}; 
    int hashLen = 0;
    hashLen = sha256(message, hash);
    NostrString msgHash = NostrString_bytesToHex(hash, hashLen);

    Utils::log("Message hash: "+msgHash);

    // Generate the schnorr sig of the messageHash
    byte messageBytes[NOSTR_DIGEST_SIZE];
    NostrString_hexToBytes(msgHash, messageBytes, NOSTR_DIGEST_SIZE);
    SchnorrSignature signature = privateKey.schnorr_sign(messageBytes);
    NostrString signatureHex = NostrString(signature);

    SignedNostrEvent signedEvent(
        msgHash, 
        pubKeyHex, 
        this->created_at, 
        this->kind, 
        this->tags, 
        this->content, 
        signatureHex
    );
    Utils::log("Signature hash " + signatureHex);


    return signedEvent;
}

bool SignedNostrEvent::verify() const{
    Serial.println("Verifying event signature");
    byte messageBytes[32];
    // fromHex(this->id, messageBytes, 32);
    NostrString_hexToBytes(this->id, messageBytes, 32);
    // Serial.println("Message hash: "+this->id);
    Utils::log("Message hash: "+this->id);

    byte pubeyBytes[32];
    NostrString_hexToBytes("02"+this->pubkey, pubeyBytes, 32);
    // fromHex("02"+this->pubkey, pubeyBytes, 32);
    // Serial.println("Pubkey: "+this->pubkey);
    Utils::log("Pubkey: "+this->pubkey);

    PublicKey pub(pubeyBytes);

    byte signatureBytes[64];
    // fromHex(this->signature, signatureBytes, 64);
    NostrString_hexToBytes(this->signature, signatureBytes, 64);
    SchnorrSignature signature(signatureBytes);
    // Serial.println("Signature: " + this->signature);
    Utils::log("Signature: " + this->signature);
    return pub.schnorr_verify(signature, messageBytes);    

}

void SignedNostrEvent::toJson(JsonObject doc) const
{
    doc["id"] = this->id;
    doc["pubkey"] = this->pubkey;
    doc["created_at"] = this->created_at;
    doc["kind"] = this->kind;
    JsonArray tags = doc["tags"].to<JsonArray>();
    this->tags.toJson(tags);
    doc["content"] = this->content;
    doc["sig"] = this->signature;    

}

void SignedNostrEvent::toSendableEvent(JsonArray doc) const
{
    doc.add("EVENT");
    JsonObject obj = doc.add<JsonObject>();
    this->toJson(obj);
}

SignedNostrEvent::SignedNostrEvent(JsonArray arr){
    if(arr[0] != "EVENT"){
        throw std::runtime_error("Invalid event type");
    }
    
    JsonObject obj;
    if(arr.size()>2){
        obj = arr[2];
        this->subid = arr[1].as<NostrString>();
    }else{
        obj = arr[1];
    }

    this->id = obj["id"].as<NostrString>();
    this->pubkey = obj["pubkey"].as<NostrString>();
    this->created_at = obj["created_at"].as<unsigned long long>();
    this->kind = obj["kind"].as<unsigned int>();
    NostrEventTags tt = NostrEventTags();

    JsonArray tags = obj["tags"].as<JsonArray>();
    for (int i = 0; i < tags.size(); i++) {
        JsonArray tag = tags[i];
        std::vector<String> value;
        for (int j = 1; j < tag.size(); j++) {
            value.push_back(tag[j].as<NostrString>());
        }
        tt.addTag(tag[0].as<NostrString>(), value);
    }
    this->tags = tt;
    this->content = obj["content"].as<NostrString>();
    this->signature = obj["sig"].as<NostrString>();
}
