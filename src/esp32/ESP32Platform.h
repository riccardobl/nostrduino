#ifndef _NOSTR_ESP32_PLATFORM_H
#define _NOSTR_ESP32_PLATFORM_H 1
#include "NostrCommon.h"

#ifdef _ESP32_BOARD_
#include "ESP32Transport.h"
#include "NostrString.h"
#include "NostrUtils.h"
#include "WiFi.h"
#include "bootloader_random.h"
#include "esp_random.h"
#include "esp_wifi.h"
#include "time.h"
#include <initializer_list>
#include <vector>
namespace nostr {
namespace esp32 {
namespace ESP32Platform {

unsigned long getUnixTimestamp() {
    time_t now;
    struct tm timeinfo;
    while (!getLocalTime(&timeinfo)) {
        Serial.println("Still waiting for ntp sync");
        delay(10);
    }
    time(&now);
    return now;
}

long int getRealRandom(long int min, long int max) {
    uint32_t rand = esp_random();
    return (rand % (max - min + 1)) + min;
}
void serialLogger(const NostrString &str) {
    Serial.println(str.c_str());
}

/**
 * Initialize the WiFi connection
 */
void initWifi(NostrString ssid, NostrString passphrase, int unused = 6) {
    esp_wifi_start();
    WiFi.persistent(false); // don't wear out the flash by writing wifi credentials to it (also fixes wifi connection on a159x36's QEMU for ESP32)
    WiFi.begin(ssid, passphrase);
    Serial.println("Connecting to " + ssid);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}

/**
 * Initialize the time service
 */
void initTime(const char *ntpServer, long unused1 = 0, int unused2 = 3600) {
    configTime(0, 0, ntpServer);
}

/**
 * Initialize platform specific code for the nostr library
 */
void initNostr(bool withLogger) {
    bootloader_random_enable();
    Utils::init();
    nostr::Utils::setUnixTimeSecondsProvider(getUnixTimestamp);
    if (withLogger)
        nostr::Utils::setLogger(serialLogger);
    nostr::Utils::setRealRandom(getRealRandom);
}

void close() {
    Utils::close();
}

/**
 * Get a platform specific transport
 */
ESP32Transport *getTransport() {
    return new nostr::esp32::ESP32Transport();
}
} // namespace ESP32Platform
} // namespace esp32
} // namespace nostr
#endif
#endif