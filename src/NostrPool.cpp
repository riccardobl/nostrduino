#include "NostrPool.h"

using namespace nostr;

void NostrRelay::send(NostrString message) {
    this->messageQueue.push_back(message);
}

void NostrRelay::processQueue()
{
    if (this->ws.isConnected() && this->messageQueue.size() > 0)
    {
        for (auto &message : this->messageQueue)
        {
            this->ws.sendTXT((uint8_t *)NostrString_toChars(message),
                             NostrString_length(message));
        }
        this->messageQueue.clear();
    }
}





void NostrPool::onEvent(NostrRelay *relay, WStype_t type, uint8_t *payload, size_t length)
{
    Serial.println("Event from relay");
    switch (type) {
        case WStype_DISCONNECTED:
            Serial.printf("[WSc] Disconnected from relay: %s\n", ".");
            break;
        case WStype_CONNECTED:
            Serial.printf("[WSc] Connected to relay: %s\n", ".");
            break;
        case WStype_TEXT: {            
            NostrString message = NostrString_fromChars((char *)payload);
            Utils::log("Received: " + message);
            DynamicJsonDocument doc(2048);
            deserializeJson(doc, message);
            if (doc.size() == 0){
                return;
            }
            if (doc[0] == "CLOSED") {
                NostrString subId = doc[1];
                NostrString reason = doc.size() > 2 ? doc[2].as<NostrString>() : "";
                std::map<NostrString, NostrSubscription>::iterator it =
                    this->subscriptions.find(subId);
                if (it != this->subscriptions.end())
                {
                    if (it->second.closeCallback != nullptr)
                    {
                        it->second.closeCallback(subId, reason);
                    }
                    this->subscriptions.erase(it);
                }
            }else if (doc[0] == "EOSE")  {
                NostrString subId = doc[1].as<NostrString>();
                NostrSubscription* sub = &this->subscriptions[subId];
                if (!sub->eose)
                {
                    sub->eose = true;
                    if (sub->eoseCallback != nullptr){
                        sub->eoseCallback(subId);
                    }             
                }          
            }else if (doc[0] == "OK"){
                NostrString eventId = doc[1].as<NostrString>();
                bool success=doc[2].as<bool>();
                NostrString message =
                    doc.size() > 3 ? doc[3].as<NostrString>() : "";
                for(int i=0;i<this->eventStatusCallbackEntries.size();i++){
                    EventStatusCallbackEntry entry = this->eventStatusCallbackEntries[i];
                    if (entry.eventId == eventId){
                        if (entry.statusCallback != nullptr){
                            entry.statusCallback(eventId, success, message);
                        }
                        this->eventStatusCallbackEntries.erase(this->eventStatusCallbackEntries.begin()+i);
                        
                        break;
                    }
                }
                

            }else if (doc[0] == "NOTICE"){
                NostrString message = doc[1].as<NostrString>();
                if(this->noticeCallback!=nullptr){
                    this->noticeCallback(relay, message);
                }            
            }else if (doc[0] == "EVENT")  {
                NostrString subId = doc[1].as<NostrString>();
                NostrSubscription sub = this->subscriptions[subId];        
                SignedNostrEvent event(doc.as<JsonArray>());
                event.stored=!sub.eose;
                if (sub.eventCallback != nullptr){
                    sub.eventCallback(subId, &event);
                }
            }
            break;
        }
        case WStype_ERROR:
            Serial.printf("[WSc] Error from relay %s: %s\n", ".", payload);
            break;
        default:
            break;
    }
}


NostrString NostrPool::subscribeMany(
    std::initializer_list<NostrString> urls,
    std::initializer_list<
        std::map<NostrString, std::initializer_list<NostrString>>>
        filters,
    NostrEventCallback eventCallback, NostrCloseCallback closeCallback,
    NostrEOSECallback eoseCallback) {
    NostrString subId = Utils::getNewSubscriptionId();
    DynamicJsonDocument doc(2048);
    JsonArray req = doc.createNestedArray("req");
    req.add("REQ");
    req.add(subId);
    for (const auto &filter : filters)
    {
        JsonObject filterObj = req.createNestedObject();
        for (const auto &pair : filter)
        {
            NostrString key = pair.first;
            JsonArray arr = filterObj.createNestedArray(key);
            bool isUint = key == "kinds" || key == "since" || key == "until" || key == "limit";
            for (const auto &value : pair.second)
            {
                if (isUint)
                {
                    NostrString vStr = value;
                    arr.add(NostrString_toInt(vStr));
                }
                else
                {
                    arr.add(value);
                }
            }
        }
    }

    NostrString json;
    serializeJson(req, json);

    if (this->subscriptions.find(subId) == this->subscriptions.end())
    {
        // if subscription does not exist, create it
        this->subscriptions[subId] = NostrSubscription();
        this->subscriptions[subId].closeCallback = closeCallback;
        this->subscriptions[subId].eoseCallback = eoseCallback;
        this->subscriptions[subId].eventCallback = eventCallback;
    
    }

    for (auto url : urls)
    {
        Utils::log("Subscribe to " + url + " with " + json);
        NostrRelay* r=this->ensureRelay(url);
        r->send(json);
    }

    return subId;
}

void NostrPool::closeSubscription(NostrString subId) {
    if(this->subscriptions.find(subId)==this->subscriptions.end()){
        // if subscription does not exist, ignore
        return;
    }

    DynamicJsonDocument doc(128);
    JsonArray req = doc.createNestedArray("req");
    req.add("CLOSE");
    req.add(subId);
    NostrString json;
    serializeJson(req, json);
    for (NostrRelay &r : this->relays) {
        r.send(json);
    }
    doc.clear();

    this->subscriptions.erase(subId);
}




NostrRelay *NostrPool::ensureRelay(NostrString url) {
    NostrRelay *relay = NULL;
    for (auto &r : this->relays)
    {
        if (r.url == url)
        {
            relay = &r;
            break;
        }
    }
    if (relay == NULL)
    {
        Utils::log("Creating new relay for " + url);
        relays.emplace_back(WebSocketsClient(), url);
        relay = &relays.back();
        NostrString url = relay->url;
        bool ssl = NostrString_startsWith(url,"wss://");
        url = NostrString_substring(url,ssl ? 6 : 5);
        NostrString host =
            NostrString_substring(url, 0, NostrString_indexOf(url, "/"));
        NostrString path =
            NostrString_substring(url, NostrString_indexOf(url, "/"));
        if (path.equals(""))
            path = "/";
        int port = ssl ? 443 : 80;
        if (NostrString_indexOf(host, ":") != -1)
        {

     
            NostrString portStr = NostrString_substring(host, NostrString_indexOf(host, ":") + 1);
            port = NostrString_toInt(portStr);
            host = NostrString_substring(host, 0, NostrString_indexOf(host, ":"));

        }

        if (ssl)
        {
            Serial.println("Connecting to " + host + " : " + port + " with SSL " + path + "...");
            relay->ws.beginSSL(host, port, path);
        }
        else
        {
            Serial.println("Connecting to " + host + " : " + port + "..." + path);
            relay->ws.begin(host, port, path);
        }
        relay->ws.setReconnectInterval(5000);
        relay->ws.onEvent([this, relay](WStype_t type, uint8_t *payload, size_t length)
                          { this->onEvent(relay, type, payload, length); });
    }

    return relay;
}

void NostrPool::disconnectRelay(NostrString url) {
    // disconnect and remove relay
    for (auto it = this->relays.begin(); it != relays.end(); it++)
    {
        if (it->url == url)
        {
            it->ws.disconnect();
            relays.erase(it);
            break;
        }
    }
}

void NostrPool::close()
{
    for (auto &relay : relays)
    {
        relay.ws.disconnect();
    }
    relays.clear();
}

void NostrPool::publish(std::initializer_list<NostrString> rs,
                        SignedNostrEvent *event,
                        NostrEventStatusCallback statusCallback) {
    DynamicJsonDocument doc(2048);
    JsonArray ev = doc.createNestedArray();
    event->toSendableEvent(ev);
    NostrString evJson;
    serializeJson(ev, evJson);
    doc.clear();
    for (auto &r : rs)
    {
        NostrRelay *relay = this->ensureRelay(r);
        Utils::log("Sending event to relay: " + r + " with payload: " + evJson);
        relay->send(evJson);
    }
    if (statusCallback){
        EventStatusCallbackEntry entry;
        entry.statusCallback = statusCallback;
        entry.timestampSeconds = Utils::unixTimeSeconds();
        entry.eventId = event->getId();
        this->eventStatusCallbackEntries.push_back(entry);
    }
}
void NostrPool::loop()
{
    if (WiFi.status() != WL_CONNECTED){
        // Serial.println("Not connected");
        return;
    }
    // Serial.printf("Relays size %d\n",relays.size());
    for (auto &relay : relays)
    {
        // Serial.println("Looping");
        relay.ws.loop();
        // Serial.println("Process queue");

        relay.processQueue();
    }

    // remove expired callback entries
    long long now = Utils::unixTimeSeconds();
    for(int i=0;i<this->eventStatusCallbackEntries.size();i++){
        EventStatusCallbackEntry entry = this->eventStatusCallbackEntries[i];
        if (now - entry.timestampSeconds > this->eventStatusTimeoutSeconds){
            this->eventStatusCallbackEntries.erase(this->eventStatusCallbackEntries.begin()+i);
        }
        i--;
    }
}

