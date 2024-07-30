
#include "NostrCommon.h"

#ifndef _NOSTR_UNOR4_PLATFORM_H
#define _NOSTR_UNOR4_PLATFORM_H
#ifdef _UNOR4_BOARD_

#include "Arduino.h"
#include "NostrString.h"
#include "RTC.h"
#include "NostrUtils.h"
#include "unor4/UNOR4Transport.h"

#include <NTPClient.h>
#include <WiFiS3.h>
#include <WiFiUdp.h>

namespace nostr {
namespace unor4 {
namespace UNOR4Platform {
unsigned long getUnixTimestamp() {
    RTCTime currentTime;
    RTC.getTime(currentTime);
    unsigned long unixTime = currentTime.getUnixTime();
    return unixTime;
}

long int getRealRandom(long int min, long int max) {
    uint32_t rand = random(min, max + 1);
    return rand;
}

void serialLogger(const NostrString &str) {
    Serial.println(str.c_str());
}

/**
 * Initialize the WiFi connection
 */
void initWifi(NostrString ssid, NostrString passphrase, int unused = 6) {
    IPAddress dns(8, 8, 8, 8);

    if (WiFi.status() == WL_NO_MODULE) {
        while (true){
            Serial.println("Communication with WiFi module failed!");
            delay(10);
        }
    }

    String fv = WiFi.firmwareVersion();
    if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
        Serial.println("Please upgrade the firmware");
    }


    int status = WL_IDLE_STATUS;

    while (status != WL_CONNECTED) {
        Serial.print("Attempting to connect to SSID: ");
        Serial.println(ssid.c_str());
        status = WiFi.begin(ssid.c_str(), passphrase.c_str());
        delay(500);
    }
    WiFi.setDNS(dns);

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
void initTime(const char *ntpServer) {
    RTC.begin();
    timeClient.setPoolServerName(ntpServer);
    timeClient.update();
    RTCTime timeToSet = RTCTime(timeClient.getEpochTime());
    RTC.setTime(timeToSet);
    
}

/**
 * Initialize platform specific code for the nostr library
 */
void initNostr(unsigned long seed, bool withLogger) {
    randomSeed(seed);
    Utils::init();
    nostr::Utils::setUnixTimeSecondsProvider(getUnixTimestamp);
    if (withLogger)
        nostr::Utils::setLogger(serialLogger);
    nostr::Utils::setRealRandom(getRealRandom);
}

/**
 * Get a platform specific transport
 */
UNOR4Transport *getTransport() {
    return new nostr::unor4::UNOR4Transport();
}

void close() {
    Utils::close();
}
} // namespace UNOR4Platform
} // namespace unor4
} // namespace nostr
#endif
#endif