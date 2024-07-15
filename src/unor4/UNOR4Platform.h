
#ifndef _NOSTR_UNOR4_PLATFORM_H
#define _NOSTR_UNOR4_PLATFORM_H 
#ifdef ARDUINO_UNOWIFIR4

#include "Arduino.h"
#include "NostrString.h"
#include "RTC.h"
#include "Utils.h"
#include "unor4/UNOR4Transport.h"

#include <NTPClient.h>
#include <WiFiS3.h>
#include <WiFiUdp.h>

namespace nostr {
namespace unor4 {
namespace UNOR4Platform {
inline unsigned long getUnixTimestamp() {
    RTCTime currentTime;
    RTC.getTime(currentTime);
    unsigned long unixTime = currentTime.getUnixTime();
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
    if (WiFi.status() == WL_NO_MODULE) {
        Serial.println("Communication with WiFi module failed!");
        while (true)
            ;
    }

    String fv = WiFi.firmwareVersion();
    if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
        Serial.println("Please upgrade the firmware");
    }

    int wifiStatus = WiFi.begin(ssid.c_str(), passphrase.c_str());

    while (wifiStatus != WL_CONNECTED) {
        Serial.print("Connecting...");
        wifiStatus = WiFi.status();
        delay(500);
    }

    Serial.println("Connected to WiFi");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}

WiFiUDP Udp;
NTPClient timeClient(Udp);
/**
 * Initialize the time service
 */
inline void initTime(const char *ntpServer, long timeZoneOffsetHours = 0) {
    RTC.begin();
    timeClient.setPoolServerName(ntpServer);
    timeClient.update();
    RTCTime timeToSet = RTCTime(timeClient.getEpochTime() + (timeZoneOffsetHours * 3600));
    RTC.setTime(timeToSet);
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
inline UNOR4Transport *getTransport() {
    return new nostr::unor4::UNOR4Transport();
}
} 
} 
} 
#endif
#endif