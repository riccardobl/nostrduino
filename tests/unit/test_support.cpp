#include "test_unit.h"

namespace unit_test {

String repeatChar(char value, size_t count) {
    String result;
    result.reserve(static_cast<unsigned int>(count));
    for (size_t i = 0; i < count; ++i) result += value;
    return result;
}

String replaceRange(const String &value, int start, int length, const String &replacement) {
    if (start < 0 || length < 0 || static_cast<unsigned int>(start + length) > value.length()) return String();
    String result = value.substring(0, start);
    result += replacement;
    result += value.substring(start + length);
    return result;
}

int hexValue(char value) {
    if (value >= '0' && value <= '9') return value - '0';
    if (value >= 'a' && value <= 'f') return value - 'a' + 10;
    if (value >= 'A' && value <= 'F') return value - 'A' + 10;
    return -1;
}

bool decodeHex(const char *input, size_t inputLength, uint8_t *output, size_t outputSize) {
    if (input == nullptr || output == nullptr || inputLength != outputSize * 2) return false;
    for (size_t i = 0; i < outputSize; ++i) {
        const int high = hexValue(input[i * 2]);
        const int low = hexValue(input[i * 2 + 1]);
        if (high < 0 || low < 0) return false;
        output[i] = static_cast<uint8_t>((high << 4) | low);
    }
    return true;
}

} // namespace unit_test
