#include "NostrEncoding.h"
#include <stdint.h>

namespace {
void wipeBytes(uint8_t *output, size_t size) {
    if (output == nullptr) return;
    volatile uint8_t *p = output;
    while (size-- > 0) *p++ = 0;
}

int hexValue(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}
}

bool nostr::decodeHexExact(const NostrString &input, uint8_t *output, size_t outputCapacity, size_t expectedBytes) {
    if (expectedBytes > outputCapacity || output == nullptr || expectedBytes > (SIZE_MAX / 2)) {
        wipeBytes(output, outputCapacity < expectedBytes ? outputCapacity : expectedBytes);
        return false;
    }
    const long inputLength = NostrString_length(input);
    const size_t expectedChars = expectedBytes * 2;
    if (inputLength < 0 || static_cast<size_t>(inputLength) != expectedChars) {
        wipeBytes(output, expectedBytes);
        return false;
    }
    for (size_t i = 0; i < expectedChars; ++i) {
        if (hexValue(input[i]) < 0) {
            wipeBytes(output, expectedBytes);
            return false;
        }
    }
    for (size_t i = 0; i < expectedBytes; ++i) {
        output[i] = static_cast<uint8_t>((hexValue(input[i * 2]) << 4) | hexValue(input[i * 2 + 1]));
    }
    return true;
}

bool nostr::decodeHexExact(const NostrString &input, uint8_t *output, size_t expectedBytes) {
    return decodeHexExact(input, output, expectedBytes, expectedBytes);
}
