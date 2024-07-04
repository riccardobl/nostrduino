/*
  NWC.ino - Arduino library for Nostr.
  Created by Riccardo Balbo <nostr@rblb.it>, July 2th 2024
  Released under MIT License
*/
#include <WebSocketsClient.h>
#include <WiFi.h>
#include <memory>

#include "ArduinoJson.h"
#include "NostrEvent.h"
#include "NostrPool.h"
#include "Utils.h"
#include "bootloader_random.h"
#include "esp_random.h"
#include "esp_wifi.h"
#include "time.h"
#include "Nip04.h"


////////////////////////
/// PLATFORM SPECIFIC HELPERS
///  Note: see initialization code in setup()
//////////////////////// 
unsigned long getUnixTimestamp()
{
    time_t now;
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo))
    {
        Serial.println("Failed to obtain time");
        return 0;
    }
    else
    {
        Serial.println("Got timestamp of " + String(now));
    }
    time(&now);
    return now;
}


long int getRealRandom(long int min, long int max){
    uint32_t rand =  esp_random();
    return (rand % (max - min + 1)) + min;
}

void serialLogger(const NostrString& str){
    Serial.println(str.c_str());
}
////////////////////////
/// END OF PLATFORM SPECIFIC HELPERS
////////////////////////


std::vector<nostr::NostrPool*> pools;
void testNIP01();
void testNIP04();
void testNIP47();

void setup()
{

    ////////////////////////
    /// INITIALIZATION
    ///   Note: you need some form of this code in your sketch
    ///         or the library will not work properly.
    ///         If you don't know what to do, just copy this code
    ////////////////////////
    // Init serial console
    Serial.begin(115200);  

    // Init platform
    // This is the init code for ESP32.
    // Unless you are porting the code to a different platform you should probably copy
    // it as it is in your sketch with all the referenced functions (getUnixTimestamp, serialLogger, getRealRandom) 
    Serial.println("Init platform");
    esp_wifi_start();
    bootloader_random_enable();
    nostr::Utils::setUnixTimeSecondsProvider(getUnixTimestamp);
    nostr::Utils::setLogger(serialLogger);
    nostr::Utils::setRealRandom(getRealRandom);
    

    // Connect to wifi
    // Set your wifi ssid and password here
    String ssid="Wokwi-GUEST";
    String password="";
    WiFi.begin(ssid, password, 6);
    Serial.println("Connecting to "+ssid);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

    // Configure NTP
    Serial.println("Configuring NTP");
    const char *ntpServer = "pool.ntp.org";
    const long gmtOffset_sec = 0;
    const int daylightOffset_sec = 3600;
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

    // Initialization is complete
    Serial.println("Ready!");

    ////////////////////////
    /// END OF INITIALIZATION
    ////////////////////////

    testNIP01();
    testNIP04();
    // testNIP47();
}



void loop(){

    // We need to call some pool methods in the main loop to make its internal loop work properly
    // it is not very important how it is done, here we keep a reference of every pool in a vector
    // but you can do it in a different way
    for(nostr::NostrPool* pool:pools){
        // Run internal loop: refresh relays, complete pending connections, send pending messages
        pool->loop();
    }
}

// Just a test for basic
// nostr functionality
void testNIP01() {
    String relay = "wss://nostr.rblb.it:7777";
    // nostr private key
    String privKey =
        "1558dadfae151555818a6aa6cf046ca3dfbb196c419efc18482479a74b74009a";

    // We need a NostrPool instance that will handle all the communication
    nostr::NostrPool *pool = new nostr::NostrPool();
    pools.push_back(
        pool);  // NB. we are adding it to this vector since we need to call
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
             {"#t", {"arduinoTest"}}}  //,
            // You can add another filter here
        },
        [&](const String &subId, nostr::SignedNostrEvent *event) {
            // This is the callback that will be called when an event is
            // received We can access the event content with event->getContent()

            // Here you should handle the event, for this test we will just
            // serialize it and print to console
            DynamicJsonDocument doc(2048);
            JsonArray arr = doc.createNestedArray("data");
            event->toSendableEvent(arr);
            String json;
            serializeJson(arr, json);

            Serial.println("Event received: " + json);
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
        }
    );

    // NB. you might want to save the subId somewhere since you are going to
    // need it to close the subscription like so: pool.closeSubscription(subId);

    // The pool will start to listen for events immediately after the next
    // loop()

    // Lets try to send an event
    // First we create an unsigned event
    nostr::UnsignedNostrEvent ev(1, "Hello, World!",
                                 nostr::Utils::unixTimeSeconds());
    // we can add some tags
    ev.getTags().addTag("t", {"arduinoTest"});
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
}



void testNIP04(){
    // Test encryption used in dms and other things

    
    String privKeyA = "1558dadfae151555818a6aa6cf046ca3dfbb196c419efc18482479a74b74009a";
    String pubKeyA = nostr::Utils::getPublicKey(privKeyA);
    String privKeyB = "4ba55ee68773c243f9bc714fac149b623e3a583f92738a0c633d38535110f6f2";
    String pubKeyB = nostr::Utils::getPublicKey(privKeyB);

    String text = "Hello world 123";
    Serial.println("Text: \"" + text+"\"");
    String encryptedText = nostr::Nip04::encrypt(privKeyA, pubKeyB, text);
    Serial.println("Encrypted text: \"" + encryptedText+"\"");
    String decryptedText = nostr::Nip04::decrypt(privKeyB, pubKeyA, encryptedText);
    Serial.println("Decrypted text: \"" + decryptedText+"\"");
    
    if(text.equals(decryptedText)){
        Serial.println("NIP04 test passed");
    }else{
        Serial.println("NIP04 test failed");
    }

}

void testNIP47(){

}