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

int base64Value(char c, bool urlSafe) {
    if (c >= 'A' && c <= 'Z') return c - 'A';
    if (c >= 'a' && c <= 'z') return c - 'a' + 26;
    if (c >= '0' && c <= '9') return c - '0' + 52;
    if (urlSafe && c == '-') return 62;
    if (urlSafe && c == '_') return 63;
    if (!urlSafe && c == '+') return 62;
    if (!urlSafe && c == '/') return 63;
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

bool nostr::validateBase64Encoding(const char *encoded, size_t encodedSize, bool noPadding,
                                   bool urlSafe, size_t &decodedSize) {
    decodedSize = 0;
    if (encoded == nullptr || encodedSize == 0 || (!noPadding && encodedSize % 4 != 0) ||
        (noPadding && encodedSize % 4 == 1)) return false;

    size_t padding = 0;
    for (size_t i = 0; i < encodedSize; ++i) {
        const char c = encoded[i];
        if (c == '=') {
            if (noPadding || i < encodedSize - 2 || ++padding > 2) return false;
        } else if (base64Value(c, urlSafe) < 0 || padding != 0) {
            return false;
        }
    }
    const size_t significantSize = encodedSize - padding;
    const size_t remainder = significantSize % 4;
    if (remainder == 1) return false;
    if ((padding == 1 || (noPadding && remainder == 3)) &&
        (base64Value(encoded[significantSize - 1], urlSafe) & 0x03) != 0) return false;
    if ((padding == 2 || (noPadding && remainder == 2)) &&
        (base64Value(encoded[significantSize - 1], urlSafe) & 0x0f) != 0) return false;

    const size_t groups = significantSize / 4;
    if (groups > SIZE_MAX / 3) return false;
    decodedSize = groups * 3 + (remainder == 0 ? 0 : remainder - 1);
    return true;
}
