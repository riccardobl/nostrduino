#include <Arduino.h>

#include "esp32/ESP32Platform.h"
#include "test_unit.h"

#define NOSTRDUINO_DISCOVERED_TEST(test_name, function_name) namespace unit_test { bool function_name(); }
#include "test_registry.generated.inc"
#undef NOSTRDUINO_DISCOVERED_TEST

namespace {

using TestFunction = bool (*)();

bool runCategory(const char *name, TestFunction function) {
    const bool passed = function();
    Serial.print("UNIT_TEST ");
    Serial.print(name);
    Serial.println(passed ? " PASS" : " FAIL");
    return passed;
}

} // namespace

void setup() {
    Serial.begin(115200);
    delay(250);
    nostr::esp32::ESP32Platform::initNostr(false, true);

    bool allPassed = true;
#define NOSTRDUINO_DISCOVERED_TEST(test_name, function_name) \
    allPassed = runCategory(test_name, unit_test::function_name) && allPassed;
#include "test_registry.generated.inc"
#undef NOSTRDUINO_DISCOVERED_TEST
    Serial.println(allPassed ? "UNIT_TESTS PASSED" : "UNIT_TESTS FAILED");
}

void loop() {
    delay(1000);
}
