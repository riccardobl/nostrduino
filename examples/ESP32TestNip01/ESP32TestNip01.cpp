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
#define PRIVKEY "1558dadfae151555818a6aa6cf046ca3dfbb196c419efc18482479a74b74009a"

std::vector<nostr::NostrPool *> pools;
void testNIP01();

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

    testNIP01();
}

void loop() {
    // We need to call some pool methods in the main loop to make its internal
    // loop work properly it is not very important how it is done, here we keep
    // a reference of every pool in a vector but you can do it in a different
    // way
    for (nostr::NostrPool *pool : pools) {
        // Run internal loop: refresh relays, complete pending connections, send
        // pending messages
        pool->loop();
    }
}

nostr::Transport *transport;

void testNIP01() {
    try {
        String relay = RELAY;
        String privKey = PRIVKEY;
        transport = nostr::esp32::ESP32Platform::getTransport();

        // We need a NostrPool instance that will handle all the communication
        nostr::NostrPool *pool = new nostr::NostrPool(transport);
        pools.push_back(pool); // NB. we are adding it to this vector since we need to call
                               // pool->loop() in the main loop to make it work properly

        // Lets subscribe to the relay
        String subId = pool->subscribeMany(
            {relay},
            {
                {// we set the filters here (see
                 // https://github.com/nostr-protocol/nips/blob/master/01.md#from-client-to-relay-sending-events-and-creating-subscriptions)
                 {"kinds", {"1"}},
                 // {"since",{"1234567890"}},
                 // {"until",{"1234567890"}},
                 // {"limit",{"10"}},
                 {"#t", {"arduinoTest"}}} //,
                // You can add another filter here
            },
            [&](const String &subId, nostr::SignedNostrEvent *event) {
                // Received events callback, we can access the event content with
                // event->getContent() Here you should handle the event, for this
                // test we will just serialize it and print to console
                JsonDocument doc;
                JsonArray arr = doc["data"].to<JsonArray>();
                event->toSendableEvent(arr);
                String json;
                serializeJson(arr, json);

                Serial.println("Event received!: " + json);

                if (event->verify()) {
                    Serial.println("Event signature is valid");
                } else {
                    Serial.println("Event signature is invalid");
                }
            },
            [&](const String &subId, const String &reason) {
                // This is the callback that will be called when the subscription is
                // closed
                Serial.println("Subscription closed: " + reason);
            },
            [&](const String &subId) {
                // This is the callback that will be called when the subscription is
                // EOSE
                Serial.println("Subscription EOSE: " + subId);
            });

        // NB. you might want to save the subId somewhere since you are going to
        // need it to close the subscription like so: pool.closeSubscription(subId);

        // The pool will start to listen for events immediately after the next
        // loop()

        // Lets try to send an event
        // First we create an unsigned event
        nostr::UnsignedNostrEvent ev(1, "Hello, World!", nostr::Utils::unixTimeSeconds());
        // we can add some tags
        ev.getTags()->addTag("t", {"arduinoTest"});
        // then we sign it with our private key and we will get a SignedNostrEvent
        // as result
        nostr::SignedNostrEvent signEv = ev.sign(privKey);
        // We can verify the event signature (this is not necessary here, but it
        // might be when dealing with received events)
        if (!signEv.verify()) {
            Serial.println("Event signature is invalid");
            return;
        } else {
            Serial.println("Event signature is valid");
        }
        // Now we can send the event
        pool->publish({relay}, &signEv);
        // The event will be sent in the next loop() call
    } catch (const std::exception &e) {
        Serial.println("Error: " + String(e.what()));
    }
}
