#ifndef _NOSTR_ESP32_PLATFORM_H
#define _NOSTR_ESP32_PLATFORM_H 1
#ifdef ESP32
#include "NostrString.h"
#include "Utils.h"
#include "bootloader_random.h"
#include "ESP32Transport.h"
#include "esp_random.h"
#include "esp_wifi.h"
#include "time.h"
#include <initializer_list>
#include <vector>
namespace nostr {
    namespace esp32{
        namespace ESP32Platform{
            inline unsigned long getUnixTimestamp() {
                time_t now;
                struct tm timeinfo;
                if (!getLocalTime(&timeinfo)) {
                    return 0;
                }
                time(&now);
                return now;
            }

            inline  long int getRealRandom(long int min, long int max) {
                uint32_t rand = esp_random();
                return (rand % (max - min + 1)) + min;
            }
            inline void serialLogger(const NostrString &str) {
                Serial.println(str.c_str());
            }

            /**
             * Initialize the WiFi connection
             */
            inline void initWifi(NostrString ssid,  NostrString passphrase, int channel = 6) {
                esp_wifi_start();
                WiFi.begin(ssid, passphrase, channel);
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
            inline void initTime(const char* ntpServer, long gmtOffset_sec = 0,
                                 int daylightOffset_sec = 3600) {              
                configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
            }

            /**
             * Initialize platform specific code for the nostr library
             */
            inline void initNostr(bool withLogger){
                bootloader_random_enable();
                nostr::Utils::setUnixTimeSecondsProvider(getUnixTimestamp);
                if(withLogger)nostr::Utils::setLogger(serialLogger);
                nostr::Utils::setRealRandom(getRealRandom);
            }

            /**
             * Get a platform specific transport 
             */
            inline ESP32Transport* getTransport(){
                return  new nostr::esp32::ESP32Transport();                
            }
        }
    }
}
#endif
#endif