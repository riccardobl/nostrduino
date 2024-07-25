
#include "Common.h"
#ifndef _NOSTR_PICOW_PLATFORM_H
#define _NOSTR_PICOW_PLATFORM_H 
#ifdef _PICOW_BOARD_

#include <SPI.h>
#include <WiFi.h>
#include "Arduino.h"
#include "NostrString.h"
#include "Utils.h"
#include "picow/PICOWTransport.h"
#include <time.h>
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
    IPAddress dns(8, 8, 8, 8);
 

    if (WiFi.status() == WL_NO_SHIELD) {
        while (true){
            Serial.println("Communication with WiFi module failed!");
        }
    }
    
    int status = WL_IDLE_STATUS;

    while (status != WL_CONNECTED) {
        Serial.print("Attempting to connect to SSID: ");
        Serial.println(ssid.c_str());
        status = WiFi.begin(ssid.c_str(), passphrase.c_str());

        delay(10000);
    }

    WiFi.setDNS(dns);

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