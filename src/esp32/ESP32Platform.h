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

inline unsigned long getUnixTimestamp() {
    time_t now;
    struct tm timeinfo;
    while (!getLocalTime(&timeinfo)) {
        Serial.println("Still waiting for ntp sync");
        delay(10);
    }
    time(&now);
    return now;
}

inline bool fillRandom(uint8_t *output, size_t outputSize) {
    if (output == nullptr && outputSize != 0) return false;
    if (outputSize == 0) return true;
    // The ESP32 hardware RNG exposes only a 32-bit state register that is
    // refilled by the physical entropy source at a limited rate; draining it
    // in one shot (esp_fill_random) can outpace the refill and produce
    // correlated words, especially when Wi-Fi/BT are off and no RF noise is
    // mixed in (see Espressif's esp_random docs). Pull one 32-bit word per
    // esp_random() call and yield ~1 ms between reads so the source can
    // refill; the yield also adds scheduler timing jitter. Same mitigation
    // as Blockstream Jade 1.0.41.
    size_t offset = 0;
    while (offset < outputSize) {
        uint32_t word = esp_random();
        for (size_t i = 0; i < sizeof(word) && offset < outputSize; ++i) {
            output[offset++] = static_cast<uint8_t>(word & 0xFF);
            word >>= 8;
        }
        if (offset < outputSize) delay(1);
    }
    return true;
}
inline void serialLogger(const NostrString &str) {
    Serial.println(str.c_str());
}

/**
 * Initialize the WiFi connection
 */
inline void initWifi(NostrString ssid, NostrString passphrase, int unused = 6) {
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
inline void initTime(const char *ntpServer, long unused1 = 0, int unused2 = 3600) {
    configTime(0, 0, ntpServer);
}

/**
 * Initialize platform specific code for the nostr library
 *
 * Set initializeRngWithoutRadio only for firmware that will never use
 * Wi-Fi/Bluetooth or the peripherals that share the offline RNG source. The
 * application must call bootloader_random_disable() before initializing RF,
 * ADC or I2S, as required by ESP-IDF.
 */
inline void initNostr(bool withLogger, bool initializeRngWithoutRadio = false) {
    if (initializeRngWithoutRadio) bootloader_random_enable();
    nostr::Utils::setRandomBytesProvider(fillRandom);
    Utils::init();
    nostr::Utils::setUnixTimeSecondsProvider(getUnixTimestamp);
    if (withLogger)
        nostr::Utils::setLogger(serialLogger);
}

inline void close() {
    Utils::close();
}

/**
 * Get a platform specific transport
 */
inline ESP32Transport *getTransport() {
    return new nostr::esp32::ESP32Transport();
}
inline ESP32Transport *getTransport(const ESP32TransportConfig &config) {
    return new nostr::esp32::ESP32Transport(config);
}
} // namespace ESP32Platform
} // namespace esp32
} // namespace nostr
#endif
#endif
