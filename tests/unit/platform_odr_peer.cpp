#include "esp32/ESP32Platform.h"

namespace unit_test {

bool platformOdrPeer() {
    return nostr::esp32::ESP32Platform::fillRandom(nullptr, 0);
}

} // namespace unit_test
