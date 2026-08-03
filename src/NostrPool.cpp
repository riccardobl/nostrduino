#include "NostrPool.h"
#include <cstring>

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
    if (NostrString_length(message) < 0 || static_cast<size_t>(NostrString_length(message)) > limits.maxWebSocketMessageBytes) return;
    JsonDocument doc;
    if (!Utils::jsonParse(&message, &doc) || !doc.is<JsonArray>() || doc.size() == 0) {
        return;
    }
    if (!doc[0].is<const char *>()) return;
    if (std::strcmp(doc[0].as<const char *>(), "CLOSED") == 0) {
        if ((doc.size() != 2 && doc.size() != 3) || !doc[1].is<const char *>() ||
            (doc.size() == 3 && !doc[2].is<const char *>()) ||
            std::strlen(doc[1].as<const char *>()) > limits.maxSubscriptionIdBytes ||
            (doc.size() == 3 && std::strlen(doc[2].as<const char *>()) > limits.maxRelayMessageBytes)) return;
        NostrString subId = doc[1].as<const char *>();
        NostrString reason = doc.size() > 2 ? doc[2].as<const char *>() : "";
        std::map<NostrString, NostrSubscription>::iterator it = this->subscriptions.find(subId);
        if (it == this->subscriptions.end()) return;
        std::map<NostrString, NostrRelaySubscriptionState>::iterator relayState =
            it->second.relayStates.find(relay->url);
        if (relayState == it->second.relayStates.end()) return;
        relayState->second.active = false;
        relayState->second.closed = true;
        relayState->second.disconnected = false;
        NostrCloseCallback closeCallback = it->second.closeCallback;
        bool allClosed = !it->second.relayStates.empty();
        for (const auto &entry : it->second.relayStates) {
            if (!entry.second.closed) {
                allClosed = false;
                break;
            }
        }
        if (allClosed) this->subscriptions.erase(subId);
        if (closeCallback != nullptr) closeCallback(subId, reason);
        if (!allClosed) this->recomputeSubscriptionEose(subId);
    } else if (std::strcmp(doc[0].as<const char *>(), "EOSE") == 0) {
        if (doc.size() != 2 || !doc[1].is<const char *>() ||
            std::strlen(doc[1].as<const char *>()) > limits.maxSubscriptionIdBytes) return;
        NostrString subId = doc[1].as<const char *>();
        std::map<NostrString, NostrSubscription>::iterator it = this->subscriptions.find(subId);
        if (it == this->subscriptions.end()) {
            return;
        }
        std::map<NostrString, NostrRelaySubscriptionState>::iterator relayState =
            it->second.relayStates.find(relay->url);
        if (relayState == it->second.relayStates.end() || !relayState->second.active ||
            relayState->second.closed || relayState->second.disconnected) return;
        relayState->second.eose = true;
        this->recomputeSubscriptionEose(subId);
    } else if (std::strcmp(doc[0].as<const char *>(), "OK") == 0) {
        if ((doc.size() != 3 && doc.size() != 4) || !doc[1].is<const char *>() || !doc[2].is<bool>() ||
            (doc.size() == 4 && !doc[3].is<const char *>()) ||
            std::strlen(doc[1].as<const char *>()) != 64 ||
            (doc.size() == 4 && std::strlen(doc[3].as<const char *>()) > limits.maxRelayMessageBytes)) return;
        uint8_t eventIdBytes[32] = {};
        if (!Utils::decodeHexExact(doc[1].as<const char *>(), eventIdBytes, sizeof(eventIdBytes))) return;
        NostrString eventId = doc[1].as<const char *>();
        bool success = doc[2].as<bool>();
        NostrString message = doc.size() > 3 ? doc[3].as<const char *>() : "";
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

    } else if (std::strcmp(doc[0].as<const char *>(), "NOTICE") == 0) {
        if (doc.size() != 2 || !doc[1].is<const char *>() ||
            std::strlen(doc[1].as<const char *>()) > limits.maxRelayMessageBytes) return;
        NostrString message = doc[1].as<const char *>();
        if (this->noticeCallback != nullptr) {
            this->noticeCallback(relay, message);
        }
    } else if (std::strcmp(doc[0].as<const char *>(), "EVENT") == 0) {
        if (doc.size() != 3 || !doc[1].is<const char *>() ||
            std::strlen(doc[1].as<const char *>()) > limits.maxSubscriptionIdBytes) return;
        NostrString subId = doc[1].as<const char *>();
        std::map<NostrString, NostrSubscription>::iterator it = this->subscriptions.find(subId);
        if (it == this->subscriptions.end()) {
            return;
        }
        NostrSubscription &sub = it->second;
        std::map<NostrString, NostrRelaySubscriptionState>::const_iterator relayState =
            sub.relayStates.find(relay->url);
        if (relayState == sub.relayStates.end() || !relayState->second.active ||
            relayState->second.closed || relayState->second.disconnected) return;
        NostrEventCallback eventCallback = sub.eventCallback;
        SignedNostrEvent event;
        if (!SignedNostrEvent::parseRelayFrame(doc.as<JsonArrayConst>(), event, limits)) return;
        if (verifyIncomingEvents && !event.verify()) return;
        event.stored = !relayState->second.eose;
        if (eventCallback != nullptr) {
            eventCallback(subId, &event);
        }
    }
    doc.clear();
}

void NostrPool::recomputeSubscriptionEose(const NostrString &subId) {
    std::map<NostrString, NostrSubscription>::iterator it = this->subscriptions.find(subId);
    if (it == this->subscriptions.end()) return;

    bool hasEligibleRelay = false;
    bool allEligibleRelaysReachedEose = true;
    for (const auto &entry : it->second.relayStates) {
        const NostrRelaySubscriptionState &state = entry.second;
        if (!state.active || state.closed || state.disconnected) continue;
        hasEligibleRelay = true;
        if (!state.eose) allEligibleRelaysReachedEose = false;
    }

    const bool reachedEose = hasEligibleRelay && allEligibleRelaysReachedEose;
    if (!reachedEose) {
        it->second.eose = false;
        return;
    }
    if (it->second.eose) return;
    it->second.eose = true;
    NostrEOSECallback callback = it->second.eoseCallback;
    if (callback != nullptr) callback(subId);
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
    NostrString subId;
    do {
        subId = Utils::getNewSubscriptionId();
        if (NostrString_length(subId) <= 0 || static_cast<size_t>(NostrString_length(subId)) > limits.maxSubscriptionIdBytes) return "";
    } while (this->subscriptions.find(subId) != this->subscriptions.end());

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

    this->subscriptions[subId] = NostrSubscription();
    this->subscriptions[subId].closeCallback = closeCallback;
    this->subscriptions[subId].eoseCallback = eoseCallback;
    this->subscriptions[subId].eventCallback = eventCallback;
    this->subscriptions[subId].request = json;
    for (auto url : urls) {
        this->subscriptions[subId].relayStates[url] = NostrRelaySubscriptionState();
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

const NostrSubscription *NostrPool::getSubscription(const NostrString &subId) const {
    auto it = this->subscriptions.find(subId);
    return it == this->subscriptions.end() ? nullptr : &it->second;
}

bool NostrPool::hasSubscription(const NostrString &subId) const {
    return this->subscriptions.find(subId) != this->subscriptions.end();
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
        relay->conn->addConnectionStatusListener([this, relay](ConnectionStatus status) {
            if (status == ConnectionStatus::DISCONNECTED) {
                relay->restoreSubscriptions = relay->connectedOnce;
                std::vector<NostrString> affectedSubscriptions;
                for (auto &entry : this->subscriptions) {
                    std::map<NostrString, NostrRelaySubscriptionState>::iterator relayState =
                        entry.second.relayStates.find(relay->url);
                    if (relayState == entry.second.relayStates.end() || relayState->second.closed) continue;
                    relayState->second.active = false;
                    relayState->second.disconnected = true;
                    affectedSubscriptions.push_back(entry.first);
                }
                for (const NostrString &subId : affectedSubscriptions) {
                    this->recomputeSubscriptionEose(subId);
                }
                return;
            }
            if (status != ConnectionStatus::CONNECTED) return;

            const bool shouldRestoreSubscriptions = relay->restoreSubscriptions;
            relay->connectedOnce = true;
            relay->restoreSubscriptions = false;
            if (!shouldRestoreSubscriptions) return;

            for (auto &entry : this->subscriptions) {
                NostrSubscription &subscription = entry.second;
                std::map<NostrString, NostrRelaySubscriptionState>::iterator relayState =
                    subscription.relayStates.find(relay->url);
                if (relayState == subscription.relayStates.end() || relayState->second.closed) continue;
                relayState->second.active = true;
                relayState->second.disconnected = false;
                relayState->second.eose = false;
                subscription.eose = false;
                relay->send(subscription.request);
            }
        });
    }
    return relay;
}

void NostrPool::disconnectRelay(NostrString url) {
    std::vector<NostrString> affectedSubscriptions;
    for (auto &entry : this->subscriptions) {
        std::map<NostrString, NostrRelaySubscriptionState>::iterator relayState =
            entry.second.relayStates.find(url);
        if (relayState == entry.second.relayStates.end() || relayState->second.closed) continue;
        relayState->second.active = false;
        relayState->second.disconnected = true;
        affectedSubscriptions.push_back(entry.first);
    }
    for (const NostrString &subId : affectedSubscriptions) {
        this->recomputeSubscriptionEose(subId);
    }
    for (size_t i = 0; i < this->relays.size(); i++) {
        if (NostrString_equals(this->relays[i]->url, url)) {
            this->transport->disconnect(this->relays[i]->conn);
            delete this->relays[i];
            this->relays.erase(this->relays.begin() + i);
            break;
        }
    }
}

void NostrPool::close() {
    Utils::log("Closing NostrPool");
    for (auto &relay : relays) {
        this->transport->disconnect(relay->conn);
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
