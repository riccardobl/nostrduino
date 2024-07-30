#include "NostrPool.h"

using namespace nostr;

void NostrRelay::send(NostrString message) {
    this->messageQueue.push_back(message);
}

void NostrRelay::processQueue() {
    if (!this->conn->isReady()) {
        return;
    }
    if (this->messageQueue.size() > 0) {
        for (auto &message : this->messageQueue) {
            this->conn->send(message);
        }
        this->messageQueue.clear();
    }
}

void NostrPool::onEvent(NostrRelay *relay, NostrString message) {
    JsonDocument doc;
    Utils::jsonParse(&message, &doc);
    if (doc.size() == 0) {
        return;
    }
    if (NostrString_equals(doc[0], "CLOSED")) {
        NostrString subId = doc[1];
        NostrString reason = doc.size() > 2 ? doc[2].as<NostrString>() : "";
        std::map<NostrString, NostrSubscription>::iterator it = this->subscriptions.find(subId);
        if (it != this->subscriptions.end()) {
            if (it->second.closeCallback != nullptr) {
                it->second.closeCallback(subId, reason);
            }
            this->subscriptions.erase(it);
        }
    } else if (NostrString_equals(doc[0], "EOSE")) {
        NostrString subId = doc[1].as<NostrString>();
        NostrSubscription *sub = &this->subscriptions[subId];
        if (!sub->eose) {
            sub->eose = true;
            if (sub->eoseCallback != nullptr) {
                sub->eoseCallback(subId);
            }
        }
    } else if (NostrString_equals(doc[0], "OK")) {
        NostrString eventId = doc[1].as<NostrString>();
        bool success = doc[2].as<bool>();
        NostrString message = doc.size() > 3 ? doc[3].as<NostrString>() : "";
        for (size_t i = 0; i < this->eventStatusCallbackEntries.size(); i++) {
            EventStatusCallbackEntry entry = this->eventStatusCallbackEntries[i];
            if (NostrString_equals(entry.eventId, eventId)) {
                if (entry.statusCallback != nullptr) {
                    entry.statusCallback(eventId, success, message);
                }
                this->eventStatusCallbackEntries.erase(this->eventStatusCallbackEntries.begin() + i);
                break;
            }
        }

    } else if (NostrString_equals(doc[0], "NOTICE")) {
        NostrString message = doc[1].as<NostrString>();
        if (this->noticeCallback != nullptr) {
            this->noticeCallback(relay, message);
        }
    } else if (NostrString_equals(doc[0], "EVENT")) {
        NostrString subId = doc[1].as<NostrString>();
        NostrSubscription sub = this->subscriptions[subId];
        SignedNostrEvent event(doc.as<JsonArray>());
        event.stored = !sub.eose;
        if (sub.eventCallback != nullptr) {
            sub.eventCallback(subId, &event);
        }
    }
    doc.clear();
}

NostrString NostrPool::subscribeMany(std::initializer_list<NostrString> urls, std::initializer_list<std::map<NostrString, std::initializer_list<NostrString>>> filters,
                                     NostrEventCallback eventCallback, NostrCloseCallback closeCallback, NostrEOSECallback eoseCallback) {
    JsonDocument doc;
    JsonArray filtersArray = doc["filtersArray"].to<JsonArray>();
    for (const auto &filter : filters) {
        JsonObject filterObj = filtersArray.add<JsonObject>();
        for (const auto &pair : filter) {
            NostrString key = pair.first;
            bool isIntList = NostrString_equals(key, "kinds");
            bool isInt = NostrString_equals(key, "since") || NostrString_equals(key, "until") || NostrString_equals(key, "limit");
            bool isFloat = false;
            bool isFloatList = false;
            bool isString = false;
            bool isPrefixed = NostrString_indexOf(key, " ") != -1;
            if (isPrefixed) {
                if(NostrString_startsWith(key,"int ")){
                    isInt = true;
                    key = NostrString_substring(key, 4);
                }else if(NostrString_startsWith(key,"int[] ")){
                    isIntList = true;
                    key = NostrString_substring(key, 6);
                }else if(NostrString_startsWith(key,"float[] ")){
                    isFloatList = true;
                    key = NostrString_substring(key, 8);
                }else if(NostrString_startsWith(key,"float ")){
                    isFloat = true;
                    key = NostrString_substring(key, 6);
                }else if(NostrString_startsWith(key,"string ")){
                    isString = true;
                    key = NostrString_substring(key, 7);
                }else if(NostrString_startsWith(key,"string[] ")){
                    key = NostrString_substring(key, 9);
                }
            }
            if (isInt) {
                std::initializer_list<NostrString> values = pair.second;
                NostrString value = *values.begin();
                filterObj[key] = NostrString_toInt(value);
            } else if(isFloat){
                std::initializer_list<NostrString> values = pair.second;
                NostrString value = *values.begin();
                filterObj[key] = NostrString_toFloat(value);            
            } else if(isString){
                std::initializer_list<NostrString> values = pair.second;
                NostrString value = *values.begin();
                filterObj[key] = value;                
            }else {
                JsonArray arr = filterObj[key].to<JsonArray>();
                for (const auto &value : pair.second) {
                    if (isIntList) {
                        NostrString vStr = value;
                        arr.add(NostrString_toInt(vStr));
                    } else if(isFloatList){
                        NostrString vStr = value;
                        arr.add(NostrString_toFloat(vStr));
                    }else {
                        arr.add(value);
                    }
                }
            }
        }
    }

    NostrString subId = this->subscribeMany(urls, filtersArray, eventCallback, closeCallback, eoseCallback);
    doc.clear();
    return subId;
}

NostrString NostrPool::subscribeMany(std::initializer_list<NostrString> urls, JsonArray filters, NostrEventCallback eventCallback, NostrCloseCallback closeCallback, NostrEOSECallback eoseCallback) {
    NostrString subId = Utils::getNewSubscriptionId();
    JsonDocument doc;
    JsonArray req = doc["req"].to<JsonArray>();
    req.add("REQ");
    req.add(subId);
    for(size_t i = 0; i < filters.size(); i++){
        JsonObject filter = filters[i].as<JsonObject>();
        req.add(filter);
    }

    NostrString json;
    Utils::jsonStringify(req, &json);
    doc.clear();

    if (this->subscriptions.find(subId) == this->subscriptions.end()) {
        // if subscription does not exist, create it
        this->subscriptions[subId] = NostrSubscription();
        this->subscriptions[subId].closeCallback = closeCallback;
        this->subscriptions[subId].eoseCallback = eoseCallback;
        this->subscriptions[subId].eventCallback = eventCallback;
    }

    for (auto url : urls) {
        NostrRelay *r = this->ensureRelay(url);
        Utils::log("Subscribe to " + url + " with " + json);
        r->send(json);
    }

    return subId;
}

void NostrPool::closeSubscription(NostrString subId) {
    Utils::log("Closing subscription " + subId);
    if (this->subscriptions.find(subId) == this->subscriptions.end()) {
        // if subscription does not exist, ignore
        return;
    }
    JsonDocument doc;
    JsonArray req = doc["req"].to<JsonArray>();
    req.add("CLOSE");
    req.add(subId);
    NostrString json;
    Utils::jsonStringify(req, &json);
    doc.clear();
    for (NostrRelay *r : this->relays) {
        r->send(json);
    }
    this->subscriptions.erase(subId);
}

NostrRelay *NostrPool::ensureRelay(NostrString url) {
    NostrRelay *relay = NULL;
    for (auto &r : this->relays) {
        if (NostrString_equals(r->url, url)) {
            relay = r;
            break;
        }
    }
    if (relay == NULL) {
        Utils::log("Creating new relay for " + url);
        Connection *conn = this->transport->connect(url);
        relay = new NostrRelay(conn, url);
        this->relays.push_back(relay);
        relay->conn->addMessageListener([this, relay](NostrString message) { this->onEvent(relay, message); });
    }
    return relay;
}

void NostrPool::disconnectRelay(NostrString url) {
    for (size_t i = 0; i < this->relays.size(); i++) {
        if (NostrString_equals(this->relays[i]->url, url)) {
            this->relays[i]->conn->disconnect();
            delete this->relays[i];
            this->relays.erase(this->relays.begin() + i);
            break;
        }
    }
}

void NostrPool::close() {
    Utils::log("Closing NostrPool");
    for (auto &relay : relays) {
        relay->conn->disconnect();
        delete relay;
    }
    relays.clear();
}

void NostrPool::publish(std::initializer_list<NostrString> rs, SignedNostrEvent *event, NostrEventStatusCallback statusCallback) {
    JsonDocument doc;
    JsonArray ev = doc.add<JsonArray>();
    event->toSendableEvent(ev);
    NostrString evJson;
    Utils::jsonStringify(ev, &evJson);
    doc.clear();
    for (auto &r : rs) {
        NostrRelay *relay = this->ensureRelay(r);
        Utils::log("Sending event to relay: " + r + " with payload: " + evJson);
        relay->send(evJson);
    }
    if (statusCallback) {
        EventStatusCallbackEntry entry;
        entry.statusCallback = statusCallback;
        entry.timestampSeconds = Utils::unixTimeSeconds();
        entry.eventId = event->getId();
        this->eventStatusCallbackEntries.push_back(entry);
    }
}
void NostrPool::loop() {
    if (!this->transport->isReady()) {
        return;
    }
    for (auto &relay : relays) {
        relay->conn->loop();
        relay->processQueue();
    }

    // remove expired callback entries
    long long now = Utils::unixTimeSeconds();
    for (size_t i = 0; i < this->eventStatusCallbackEntries.size(); i++) {
        EventStatusCallbackEntry entry = this->eventStatusCallbackEntries[i];
        if (now - entry.timestampSeconds > this->eventStatusTimeoutSeconds) {
            this->eventStatusCallbackEntries.erase(this->eventStatusCallbackEntries.begin() + i);
            i--;
        }
    }
}

std::vector<NostrString> NostrPool::getRelays() {
    std::vector<NostrString> urls;
    for (auto &r : this->relays) {
        urls.push_back(r->url);
    }
    return urls;
}

std::vector<NostrRelay *> *NostrPool::getConnectedRelays() {
    return &this->relays;
}
