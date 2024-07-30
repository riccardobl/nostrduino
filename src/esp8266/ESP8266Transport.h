#ifndef _NOSTR_ESP8266_TRANSPORT_H
#define _NOSTR_ESP8266_TRANSPORT_H 1
#include "NostrCommon.h"

#ifdef _ESP8266_BOARD_

#include "esp32/ESP32Transport.h"

namespace nostr {
namespace esp8266 {
typedef nostr::esp32::ESP32Connection ESP8266Connection;
typedef nostr::esp32::ESP32Transport ESP8266Transport;
} // namespace esp8266
} // namespace nostr
#endif
#endif
