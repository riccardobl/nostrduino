#include "test_unit.h"

#include "esp32/ESP32Platform.h"

namespace unit_test {

bool platformOdrPeer();

bool testPlatformOdr() {
    CHECK(nostr::esp32::ESP32Platform::fillRandom(nullptr, 0));
    CHECK(platformOdrPeer());
    return true;
}

} // namespace unit_test
