#include "esp32/ESP32Platform.h"

#include <Arduino.h>

#include "ArduinoJson.h"
#include "NostrEvent.h"
#include "NostrPool.h"
#include "NostrTransport.h"
#include "NostrUtils.h"
#include "time.h"

#define WIFI_SSID "Wokwi-GUEST"
#define WIFI_PASS ""

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
    nostr::esp32::ESP32Platform::initWifi(WIFI_SSID, WIFI_PASS);

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
    try {
        String relay = RELAY;
        transport = nostr::esp32::ESP32Platform::getTransport();
        nostr::NostrPool *pool = new nostr::NostrPool(transport);
        pools.push_back(pool);
        String subId = pool->subscribeMany(
            {relay},
            {{
                {"kinds", {"1"}}, {"since", {"1626023056"}}, {"until", {"1846947856"}}, {"limit", {"10"}},
                // Filters defined in NIP01 are automatically converted to the correct type
                // however this library support non-NIP01 filters as well, but you might need to
                // specify their type manually, if unspecified the code assumes string[]:
                // eg. {"int newFilter", {"1"}}
                // eg. {"int[] newFilter2", {"1", "2"}}
                // eg. {"float newFilter3", {"1.1"}}
                // eg. {"float[] newFilter4", {"1.1", "2.2"}}
                // eg. {"string newFilter5", {"hello"}}
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

        std::vector<nostr::NostrRelay *> *relays = pool->getConnectedRelays();
        for (nostr::NostrRelay *relay : *relays) {
            Serial.println("Registering to connection events of: " + relay->getUrl());
            relay->getConnection()->addConnectionStatusListener([&](const nostr::ConnectionStatus &status) {
                String sstatus = "UNKNOWN";
                if (status == nostr::ConnectionStatus::CONNECTED) {
                    sstatus = "CONNECTED";
                } else if (status == nostr::ConnectionStatus::DISCONNECTED) {
                    sstatus = "DISCONNECTED";
                } else if (status == nostr::ConnectionStatus::ERROR) {
                    sstatus = "ERROR";
                }
                Serial.println("Connection status changed: " + sstatus);
            });
        }
    } catch (const std::exception &e) {
        Serial.println("Error: " + String(e.what()));
    }
}
