
#include "Common.h"
#ifndef _NOSTR_PICOW_PLATFORM_H
#define _NOSTR_PICOW_PLATFORM_H 
#ifdef _PICOW_BOARD_

#include "Arduino.h"
#include "NostrString.h"
#include "Utils.h"
#include "picow/PICOWTransport.h"
#include <time.h>

#include <WiFi.h>

namespace nostr {
namespace picow {
namespace PICOWPlatform {
inline unsigned long getUnixTimestamp() {
    time_t t = time(nullptr);
    unsigned long unixTime = t;
    return unixTime;
}

inline long int getRealRandom(long int min, long int max) {
    uint32_t rand = random(min, max + 1);
    return rand;
}

inline void serialLogger(const NostrString &str) {
    Serial.println(str.c_str());
}

/**
 * Initialize the WiFi connection
 */
inline void initWifi(NostrString ssid, NostrString passphrase, int channel = 6) {
    WiFi.mode(WIFI_STA);
    if (WiFi.status() == WL_NO_MODULE) {
        while (true){
            Serial.println("Communication with WiFi module failed!");
        }
    }

    String fv = WiFi.firmwareVersion();
    if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
        Serial.println("Please upgrade the firmware");
    }

    int wifiStatus = WiFi.begin(ssid.c_str(), passphrase.c_str());

    while (wifiStatus != WL_CONNECTED) {
        Serial.printf("Connecting... status: %d\n ",  wifiStatus);
        wifiStatus = WiFi.status();
        delay(500);
    }

    Serial.println("Connected to WiFi");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}


/**
 * Initialize the time service
 */
inline void initTime(const char *ntpServer, long timeZoneOffsetHours = 0) {
    NTP.begin(ntpServer);
    NTP.waitSet();
}

/**
 * Initialize platform specific code for the nostr library
 */
inline void initNostr(unsigned long seed, bool withLogger) {
    randomSeed(seed);
    nostr::Utils::setUnixTimeSecondsProvider(getUnixTimestamp);
    if (withLogger)
        nostr::Utils::setLogger(serialLogger);
    nostr::Utils::setRealRandom(getRealRandom);
}

/**
 * Get a platform specific transport
 */
inline PICOWTransport *getTransport() {
    return new nostr::picow::PICOWTransport();
}
} 
} 
} 
#endif
#endif