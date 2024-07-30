#include "esp32/ESP32Platform.h"

#include <Arduino.h>

#include "Nip04.h"
#include "NostrTransport.h"
#include "services/NWC.h"

#define WIFI_SSID "Wokwi-GUEST"
#define WIFI_PASS ""

void testNIP04();

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

    Serial.println("Init time");
    nostr::esp32::ESP32Platform::initTime("pool.ntp.org");

    Serial.println("Init Nostr");
    nostr::esp32::ESP32Platform::initNostr(true);

    Serial.println("Ready!");

    ////////////////////////
    /// END OF INITIALIZATION
    ////////////////////////

    testNIP04();
}

void testNIP04() {
    String privKeyA = "1558dadfae151555818a6aa6cf046ca3dfbb196c419efc18482479a74b74009a";
    String pubKeyA = nostr::Utils::getPublicKey(privKeyA);
    String privKeyB = "4ba55ee68773c243f9bc714fac149b623e3a583f92738a0c633d38535110f6f2";
    String pubKeyB = nostr::Utils::getPublicKey(privKeyB);

    String text = "Hello world 123";
    Serial.println("Text: \"" + text + "\"");
    nostr::Nip04 nip04;
    String encryptedText = nip04.encrypt(privKeyA, pubKeyB, text);
    Serial.println("Encrypted text: \"" + encryptedText + "\"");
    String decryptedText = nip04.decrypt(privKeyB, pubKeyA, encryptedText);
    Serial.println("Decrypted text: \"" + decryptedText + "\"");

    if (text.equals(decryptedText)) {
        Serial.println("NIP04 test passed");
    } else {
        Serial.println("NIP04 test failed");
    }
}

void loop() {}
