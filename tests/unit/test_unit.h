#ifndef NOSTRDUINO_UNIT_TEST_H
#define NOSTRDUINO_UNIT_TEST_H

#include <Arduino.h>
#include <cstddef>
#include <cstdint>

#define CHECK(condition) \
    do { \
        if (!(condition)) { \
            Serial.print("UNIT_ASSERT "); \
            Serial.print(__FILE__); \
            Serial.print(':'); \
            Serial.print(__LINE__); \
            Serial.print(' '); \
            Serial.println(#condition); \
            return false; \
        } \
    } while (false)

namespace unit_test {

String repeatChar(char value, size_t count);
String replaceRange(const String &value, int start, int length, const String &replacement);
bool decodeHex(const char *input, size_t inputLength, uint8_t *output, size_t outputSize);

} // namespace unit_test

#endif
