#ifndef _NOSTR_COMMON_BOARD_H
#define _NOSTR_COMMON_BOARD_H 1

#define _NANO_BOARD_

#ifdef ESP32
#define _ESP32_BOARD_
#endif

#if defined(ARDUINO_NANO_RP2040_CONNECT) || defined(ARDUINO_NANO33_IOT) 
#define _NANO_BOARD_

#define WEBSOCKETS_USE_WIFININA true
#define WEBSOCKETS_WIFININA_USE_RP2040 true
#define USE_WIFI_NINA true
#define WIFININA_USE_RP2040 true

#include <WiFiNINA_Generic.h>

#endif

#if defined(ARDUINO_UNOWIFIR4)
#define _UNOR4_BOARD_
#endif

#endif