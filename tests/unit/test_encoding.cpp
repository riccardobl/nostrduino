#include "test_unit.h"

#include <cstring>

#include "NostrEncoding.h"
#include "NostrUtils.h"

namespace unit_test {

bool expectRejected(const String &value) {
    uint8_t output[32];
    for (uint8_t &byte : output) byte = 0xa5;
    CHECK(!nostr::decodeHexExact(value, output, sizeof(output)));
    for (uint8_t byte : output) CHECK(byte == 0);
    return true;
}

bool testEncoding() {
    uint8_t output[32] = {};
    CHECK(nostr::decodeHexExact(repeatChar('a', 64), output, sizeof(output)));
    CHECK(output[0] == 0xaa && output[31] == 0xaa);
    CHECK(nostr::decodeHexExact(repeatChar('F', 64), output, sizeof(output)));
    CHECK(output[0] == 0xff && output[31] == 0xff);

    const size_t rejectedLengths[] = {0, 62, 63, 65, 66, 4096};
    for (size_t length : rejectedLengths) CHECK(expectRejected(repeatChar('a', length)));
    CHECK(expectRejected(String("0x") + repeatChar('a', 62)));
    CHECK(expectRejected(String(" ") + repeatChar('a', 63)));
    CHECK(expectRejected(repeatChar('a', 63) + " "));

    String invalid = repeatChar('a', 64);
    invalid.setCharAt(0, 'g');
    CHECK(expectRejected(invalid));
    invalid = repeatChar('a', 64);
    invalid.setCharAt(32, '/');
    CHECK(expectRejected(invalid));
    invalid = repeatChar('a', 64);
    invalid.setCharAt(63, 'z');
    CHECK(expectRejected(invalid));

    uint8_t smallOutput[32];
    for (uint8_t &byte : smallOutput) byte = 0xa5;
    CHECK(!nostr::decodeHexExact(repeatChar('a', 64), smallOutput, 31, 32));
    for (uint8_t byte : smallOutput) CHECK(byte == 0 || byte == 0xa5);

    size_t decodedSize = 0;
    CHECK(nostr::validateBase64Encoding("TWFu", 4, false, false, decodedSize) && decodedSize == 3);
    CHECK(nostr::validateBase64Encoding("TWE=", 4, false, false, decodedSize) && decodedSize == 2);
    CHECK(nostr::validateBase64Encoding("TQ==", 4, false, false, decodedSize) && decodedSize == 1);
    CHECK(nostr::validateBase64Encoding("-_8", 3, true, true, decodedSize) && decodedSize == 2);
    const char *invalidBase64[] = {"", "A", "TQ=", "TQ===", "T=Q=", "TQ==x",
                                   "TR==", "TWF=", "TQ==\n", "TQ-_"};
    for (const char *value : invalidBase64) {
        CHECK(!nostr::validateBase64Encoding(value, std::strlen(value), false, false, decodedSize));
    }
    CHECK(!nostr::validateBase64Encoding("+/8", 3, true, true, decodedSize));
    CHECK(!nostr::validateBase64Encoding("-_8=", 4, true, true, decodedSize));

    CHECK(nostr::Utils::toHex(output, 3) == "ffffff");
    CHECK(nostr::Utils::hexToBase64("4d616e") == "TWFu");
    CHECK(nostr::Utils::base64ToHex("TWFu") == "4d616e");
    CHECK(nostr::Utils::hexToBase64("bad") == "");
    CHECK(nostr::Utils::base64ToHex("!!!!") == "");
    uint8_t raw[3] = {};
    CHECK(nostr::Utils::fromHex("4d616e", raw, sizeof(raw)) == sizeof(raw));
    CHECK(std::memcmp(raw, "Man", sizeof(raw)) == 0);
    CHECK(nostr::Utils::fromBase64("TWFu", 4, raw, sizeof(raw), BASE64_STANDARD) == sizeof(raw));
    uint8_t digest[32] = {};
    CHECK(nostr::Utils::sha256("abc", digest) == 32);
    CHECK(nostr::Utils::toHex(digest, sizeof(digest)) ==
          "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad");
    return true;
}

} // namespace unit_test
