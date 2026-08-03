#include "test_unit.h"

#include <cstring>

#include "Nip04.h"
#include "NostrAES.h"
#include "NostrLimits.h"
#include "NostrUtils.h"
#include "crypto/UBitcoinBackend.h"

namespace {
bool failNip04Allocation = false;
}

extern "C" bool nostrduinoUnitTestFailNip04Allocation() {
    return failNip04Allocation;
}

namespace unit_test {

String encodeBase64(const uint8_t *bytes, size_t size) {
    char encoded[128] = {};
    const size_t encodedSize = nostr::Utils::toBase64(bytes, size, encoded, sizeof(encoded), BASE64_STANDARD);
    return encodedSize == 0 && size != 0 ? String() : String(encoded).substring(0, encodedSize);
}

String encryptedBlock(const uint8_t key[32], const uint8_t plaintext[16]) {
    uint8_t iv[16] = {};
    uint8_t block[16] = {};
    std::memcpy(block, plaintext, sizeof(block));
    AES_ctx context = {};
    AES_init_ctx_iv(&context, key, iv);
    AES_CBC_encrypt_buffer(&context, block, sizeof(block));
    return encodeBase64(block, sizeof(block)) + "?iv=" + encodeBase64(iv, sizeof(iv));
}

bool testNip04() {
    String privateKey = "0000000000000000000000000000000000000000000000000000000000000001";
    String publicKey = "79BE667EF9DCBBAC55A06295CE870B07029BFCDB2DCE28D959F2815B16F81798";
    nostr::Nip04 nip04;
    const String plaintext = "NIP-04 test with binary-safe framing";
    const String encrypted = nip04.encrypt(privateKey, publicKey, plaintext);
    CHECK(encrypted.indexOf("?iv=") > 0);
    CHECK(nip04.decrypt(privateKey, publicKey, encrypted) == plaintext);
    CHECK(nip04.decrypt(privateKey, publicKey, encrypted + "?iv=AAAA") == "");
    CHECK(nip04.decrypt(privateKey, publicKey, encrypted.substring(0, encrypted.indexOf("?iv=")) + "?iv=!!!!") == "");
    CHECK(nip04.decrypt(privateKey, publicKey, encrypted.substring(0, encrypted.indexOf("?iv=")) + "?iv=AAAA") == "");
    CHECK(nip04.decrypt(privateKey, publicKey, "AAAA") == "");
    CHECK(nip04.decrypt(privateKey, publicKey, "AAAA?iv=AAAAAAAAAAAAAAAAAAAAAA==") == "");

    uint8_t sharedKey[32] = {};
    uint8_t privateBytes[32] = {};
    uint8_t publicBytes[32] = {};
    CHECK(decodeHex(privateKey.c_str(), privateKey.length(), privateBytes, sizeof(privateBytes)));
    CHECK(decodeHex(publicKey.c_str(), publicKey.length(), publicBytes, sizeof(publicBytes)));
    CHECK(nostr::crypto::UBitcoinBackend::ecdhRawX(privateBytes, publicBytes, sharedKey));
    uint8_t paddingZero[16] = {};
    CHECK(nip04.decrypt(privateKey, publicKey, encryptedBlock(sharedKey, paddingZero)) == "");
    uint8_t paddingTooLarge[16] = {};
    paddingTooLarge[15] = 17;
    CHECK(nip04.decrypt(privateKey, publicKey, encryptedBlock(sharedKey, paddingTooLarge)) == "");
    uint8_t paddingMismatch[16] = {};
    paddingMismatch[14] = 1;
    paddingMismatch[15] = 2;
    CHECK(nip04.decrypt(privateKey, publicKey, encryptedBlock(sharedKey, paddingMismatch)) == "");

    nostr::NostrLimits smallLimits;
    smallLimits.maxNip04PlaintextBytes = 15;
    smallLimits.maxNip04CiphertextBytes = 16;
    nostr::Nip04 bounded(smallLimits);
    const String boundaryPlaintext = repeatChar('x', 15);
    const String boundaryEncrypted = bounded.encrypt(privateKey, publicKey, boundaryPlaintext);
    CHECK(boundaryEncrypted.length() == 52);
    CHECK(bounded.decrypt(privateKey, publicKey, boundaryEncrypted) == boundaryPlaintext);
    CHECK(bounded.encrypt(privateKey, publicKey, repeatChar('x', 16)) == "");
    CHECK(bounded.decrypt(privateKey, publicKey, encrypted) == "");

    const auto randomBytesProvider = nostr::Utils::randomBytesProvider;
    CHECK(nostr::crypto::UBitcoinBackend::initialize([](uint8_t *output, size_t size) {
        if (output == nullptr && size != 0) return false;
        if (size != 0) std::memset(output, 0x42, size);
        return true;
    }));
    nostr::Utils::setRandomBytesProvider([](uint8_t *, size_t) { return false; });
    CHECK(nip04.encrypt(privateKey, publicKey, plaintext) == "");
    nostr::Utils::setRandomBytesProvider(randomBytesProvider);

    CHECK(nostr::crypto::UBitcoinBackend::initialize([](uint8_t *, size_t) { return false; }));
    CHECK(nip04.encrypt(privateKey, publicKey, plaintext) == "");
    CHECK(nip04.decrypt(privateKey, publicKey, encrypted) == "");
    nostr::Utils::init();
    CHECK(nip04.decrypt(privateKey, publicKey, encrypted) == plaintext);

    failNip04Allocation = true;
    CHECK(nip04.encrypt(privateKey, publicKey, "x") == "");
    failNip04Allocation = false;

    String invalidPrivate = "private";
    String invalidPublic = "public";
    CHECK(nip04.encrypt(invalidPrivate, invalidPublic, "x") == "");
    CHECK(nip04.decrypt(invalidPrivate, invalidPublic, encrypted) == "");
    return true;
}

} // namespace unit_test
