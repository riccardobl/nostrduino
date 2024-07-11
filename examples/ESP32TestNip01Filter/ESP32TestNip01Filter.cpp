#include "ArduinoJson.h"
#include "NostrEvent.h"
#include "NostrPool.h"
#include "Transport.h"
#include "Utils.h"
#include "esp32/ESP32Platform.h"
#include "time.h"

#define WIFI_SSID "Wokwi-GUEST"
#define WIFI_PASS ""
#define WIFI_CHANNEL 6

#define RELAY "wss://nostr.rblb.it:7777"

std::vector<nostr::NostrPool *> pools;
void testNIP01Filter();

void setup() {
    ////////////////////////
    /// INITIALIZATION
    ///   Note: you need some form of this code in your sketch
    ///         or the library will not work properly.
    ///         If you don't know what to do, just copy this code.
    ///         If you are initializing time and wifi somewhere else in your
    ///         code, you can just omit the related lines here and call only
    ///         initNostr
    ////////////////////////
    Serial.begin(115200);

    Serial.println("Init wifi");
    nostr::esp32::ESP32Platform::initWifi(WIFI_SSID, WIFI_PASS, WIFI_CHANNEL);

    Serial.println("Init time");
    nostr::esp32::ESP32Platform::initTime("pool.ntp.org");

    Serial.println("Init Nostr");
    nostr::esp32::ESP32Platform::initNostr(true);

    Serial.println("Ready!");

    ////////////////////////
    /// END OF INITIALIZATION
    ////////////////////////

    testNIP01Filter();
}

void loop() {    
    for (nostr::NostrPool *pool : pools) {
        pool->loop();
    }
}

nostr::Transport *transport;

void testNIP01Filter() {
    try{
        String relay = RELAY;
        transport = nostr::esp32::ESP32Platform::getTransport();
        nostr::NostrPool *pool = new nostr::NostrPool(transport);
        pools.push_back(pool);
        String subId = pool->subscribeMany(
            {relay},
            {{
                {"kinds", {"1"}},
                {"since", {"1626023056"}},
                {"until", {"1846947856"}},
                {"limit", {"10"}},
            }},
            [&](const String &subId, nostr::SignedNostrEvent *event) {               
                JsonDocument doc;
                JsonArray arr = doc["data"].to<JsonArray>();
                event->toSendableEvent(arr);
                String json;
                serializeJson(arr, json);
                Serial.println("Event received: " + json);
            },
            [&](const String &subId, const String &reason) { Serial.println("Subscription closed: " + reason); }, [&](const String &subId) { Serial.println("Subscription EOSE: " + subId); });
    } catch (const std::exception &e) {
        Serial.println("Error: " + String(e.what()));
    }
}
