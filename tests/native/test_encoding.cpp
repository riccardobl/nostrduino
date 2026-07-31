#include "NostrEncoding.h"
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <string>

static std::string hexString(size_t length, char value = 'a') {
    return std::string(length, value);
}

static void expectRejected(const std::string &value) {
    uint8_t output[32];
    std::fill(std::begin(output), std::end(output), 0xa5);
    assert(!nostr::decodeHexExact(value, output, sizeof(output)));
    for (uint8_t byte : output) assert(byte == 0);
}

int main() {
    uint8_t output[32] = {};
    assert(nostr::decodeHexExact(hexString(64), output, sizeof(output)));
    assert(output[0] == 0xaa && output[31] == 0xaa);
    assert(nostr::decodeHexExact(hexString(64, 'F'), output, sizeof(output)));
    assert(output[0] == 0xff && output[31] == 0xff);

    for (size_t length : {size_t(0), size_t(62), size_t(63), size_t(65), size_t(66), size_t(1000000)}) {
        expectRejected(hexString(length));
    }
    expectRejected("0x" + hexString(62));
    expectRejected(" " + hexString(63));
    expectRejected(hexString(63) + " ");

    std::string invalid = hexString(64);
    invalid[0] = 'g';
    expectRejected(invalid);
    invalid = hexString(64);
    invalid[32] = '/';
    expectRejected(invalid);
    invalid = hexString(64);
    invalid[63] = 'z';
    expectRejected(invalid);

    uint8_t smallOutput[32];
    std::fill(std::begin(smallOutput), std::end(smallOutput), 0xa5);
    assert(!nostr::decodeHexExact(hexString(64), smallOutput, 31, 32));
    for (uint8_t byte : smallOutput) assert(byte == 0 || byte == 0xa5);

    return 0;
}
