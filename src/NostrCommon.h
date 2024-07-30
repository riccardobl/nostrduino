#include <Arduino.h>
#include "NostrString.h"
#ifndef _NOSTR_COMMON_BOARD_H
#define _NOSTR_COMMON_BOARD_H 1

#if  defined(ESP32)
#define _ESP32_BOARD_
#endif

#if defined(ESP8266)
#define _ESP8266_BOARD_

#endif

#if defined(ARDUINO_NANO_RP2040_CONNECT) || defined(ARDUINO_NANO33_IOT) 
#define _NANO_BOARD_

#define WEBSOCKETS_USE_RP2040W true
#define USE_RP2040W_WIFI true
#define USE_WIFI_NINA false

#include <WiFiNINA_Generic.h>

#endif

#if defined(ARDUINO_ARCH_MBED_RP2040) || defined(ARDUINO_ARCH_RP2040) || defined(ARDUINO_RASPBERRY_PI_PICO_W)
#define _PICOW_BOARD_
#endif


#if defined(ARDUINO_UNOWIFIR4)
#define _UNOR4_BOARD_
#endif


#endif