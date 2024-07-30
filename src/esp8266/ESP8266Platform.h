#ifndef _NOSTR_ESP8266_PLATFORM_H
#define _NOSTR_ESP8266_PLATFORM_H 1

#include "NostrCommon.h"

#ifdef _ESP8266_BOARD_

#include "ESP8266Transport.h"
#include "NostrString.h"
#include "NostrUtils.h"
#include "time.h"
#include <ESP8266HTTPClient.h>
#include <ESP8266TrueRandom.h>
#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <initializer_list>
#include <vector>

namespace nostr {
namespace esp8266 {
namespace ESP8266Platform {
NTPClient *timeClient = NULL;
WiFiUDP ntpUDP;
String ntpServer = "";

unsigned long getUnixTimestamp() {
    if (ntpServer == "")
        throw std::runtime_error("NTP server not set");
    timeClient->update();
    return timeClient->getEpochTime();
}

long int getRealRandom(long int min, long int max) {
    return ESP8266TrueRandom.random(min, max + 1);
}
void serialLogger(const NostrString &str) {
    Serial.println(str.c_str());
}

/**
 * Initialize the WiFi connection
 */
void initWifi(NostrString ssid, NostrString passphrase, int channel = 6) {
    WiFi.begin(ssid, passphrase, channel);
    Serial.println("Connecting to " + ssid);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        yield();
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
void initTime(const char *s, long gmtOffset_sec = 0, int daylightOffset_sec = 3600) {
    ntpServer = s;
    timeClient = new NTPClient(ntpUDP, ntpServer.c_str(), 0);
    timeClient->begin();
    timeClient->update();
    while (timeClient->getEpochTime() < 1000000000) {
        Serial.println("Still waiting for ntp sync");
        timeClient->update();
        delay(1000);
    }
}

void close() {
    if (timeClient != NULL) {
        timeClient->end();
        delete timeClient;
    }
    Utils::close();
}

/**
 * Initialize platform specific code for the nostr library
 */
void initNostr(bool withLogger) {
    Utils::init();

    nostr::Utils::setUnixTimeSecondsProvider(getUnixTimestamp);
    if (withLogger)
        nostr::Utils::setLogger(serialLogger);
    nostr::Utils::setRealRandom(getRealRandom);
}

/**
 * Get a platform specific transport
 */
ESP8266Transport *getTransport() {
    return new nostr::esp8266::ESP8266Transport();
}
} // namespace ESP8266Platform
} // namespace esp8266
} // namespace nostr
#endif
#endif