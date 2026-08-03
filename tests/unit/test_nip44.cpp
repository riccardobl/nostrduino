#include "test_unit.h"

#include <cstring>
#include <vector>

#include "Nip44.h"
#include "NostrUtils.h"

namespace unit_test {

bool testNip44() {
    const String conversationKey1 = "3dfef0ce2a4d80a25e7a328accf73448ef67096f65f79588e358d9a0eb9013f1";
    const String conversationKey2 = "4d14f36e81b8452128da64fe6f1eae873baae2f444b02c950b90e43553f2178b";
    const String conversationKey3 = "9c00b769d5f54d02bf175b7284a1cbd28b6911b06cda6666b2243561ac96bad7";
    const String secret1 = "315e59ff51cb9209768cf7da80791ddcaae56ac9775eb25b6dee1234bc5d2268";
    const String public2 = "c2f9d9948dc8c7c38321e4b85c8558872eafa0641cd269db76848a6073e69133";
    CHECK(nostr::Nip44::conversationKey(secret1, public2) == conversationKey1);
    CHECK(nostr::Nip44::conversationKey(
              "a1e37752c9fdc1273be53f68c5f74be7c8905728e8de75800b94262f9497c86e",
              "03bb7947065dde12ba991ea045132581d0954f042c84e06d8c00066e23c1a800") == conversationKey2);
    CHECK(nostr::Nip44::conversationKey(
              "98a5902fd67518a0c900f0fb62158f278f94a21d6f9d33d30cd3091195500311",
              "aae65c15f98e5e677b5050de82e3aba47a6fe49b3dab7863cf35d9478ba9f7d1") == conversationKey3);

    const size_t paddingVectors[][2] = {
        {16, 32}, {32, 32}, {33, 64}, {37, 64}, {45, 64}, {49, 64}, {64, 64},
        {65, 96}, {100, 128}, {111, 128}, {200, 224}, {250, 256}, {320, 320},
        {383, 384}, {384, 384}, {400, 448}, {500, 512}, {512, 512}, {515, 640},
        {700, 768}, {800, 896}, {900, 1024}, {1020, 1024}, {65535, 65536},
        {65536, 65536}, {65537, 81920}
    };
    for (const auto &vector : paddingVectors) CHECK(nostr::Nip44::paddedLength(vector[0]) == vector[1]);
    CHECK(nostr::Nip44::paddedLength(0) == 0);

    const String nonce1 = "0000000000000000000000000000000000000000000000000000000000000001";
    const String payload1 = "AgAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAABee0G5VSK0/9YypIObAtDKfYEAjD35uVkHyB0F4DwrcNaCXlCWZKaArsGrY6M9wnuTMxWfp1RTN9Xga8no+kF5Vsb";
    const String encrypted1 = nostr::Nip44::encryptWithConversationKey("a", "c41c775356fd92eadc63ff5a0dc1da211b268cbea22316767095b2871ea1412d", nonce1);
    CHECK(encrypted1 == payload1);
    CHECK(nostr::Nip44::decryptWithConversationKey(payload1,
              "c41c775356fd92eadc63ff5a0dc1da211b268cbea22316767095b2871ea1412d") == "a");

    const String payload2 = "AvAAAAAAAAAAAAAAAAAAAPAAAAAAAAAAAAAAAAAAAAAPSKSK6is9ngkX2+cSq85Th16oRTISAOfhStnixqZziKMDvB0QQzgFZdjLTPicCJaV8nDITO+QfaQ61+KbWQIOO2Yj";
    const String encrypted2 = nostr::Nip44::encryptWithConversationKey(
        "🍕🫃", "c41c775356fd92eadc63ff5a0dc1da211b268cbea22316767095b2871ea1412d",
        "f00000000000000000000000000000f00000000000000000000000000000000f");
    CHECK(encrypted2 == payload2);
    CHECK(nostr::Nip44::decryptWithConversationKey(payload2,
              "c41c775356fd92eadc63ff5a0dc1da211b268cbea22316767095b2871ea1412d") == "🍕🫃");

    const String payload3 = "ArY1I2xC2yDwIbuNHN/1ynXdGgzHLqdCrXUPMwELJPc7s7JqlCMJBAIIjfkpHReBPXeoMCyuClwgbT419jUWU1PwaNl4FEQYKCDKVJz+97Mp3K+Q2YGa77B6gpxB/lr1QgoqpDf7wDVrDmOqGoiPjWDqy8KzLueKDcm9BVP8xeTJIxs=";
    const String encrypted3 = nostr::Nip44::encryptWithConversationKey(
        "表ポあA鷗ŒéＢ逍Üßªąñ丂㐀𠀀",
        "3e2b52a63be47d34fe0a80e34e73d436d6963bc8f39827f327057a9986c20a45",
        "b635236c42db20f021bb8d1cdff5ca75dd1a0cc72ea742ad750f33010b24f73b");
    CHECK(encrypted3 == payload3);
    CHECK(nostr::Nip44::decryptWithConversationKey(payload3,
              "3e2b52a63be47d34fe0a80e34e73d436d6963bc8f39827f327057a9986c20a45") ==
          "表ポあA鷗ŒéＢ逍Üßªąñ丂㐀𠀀");

    uint8_t extendedConversationKey[32] = {};
    uint8_t extendedNonce[32] = {};
    CHECK(decodeHex("c41c775356fd92eadc63ff5a0dc1da211b268cbea22316767095b2871ea1412d", 64,
                    extendedConversationKey, sizeof(extendedConversationKey)));
    CHECK(decodeHex(nonce1.c_str(), nonce1.length(), extendedNonce, sizeof(extendedNonce)));
    const struct {
        size_t plaintextLength;
        const char *plaintextSha256;
        const char *payloadSha256;
    } extendedVectors[] = {
        {65535, "6e1bebca6a8229364a162a72ef064826c4cd7457bf54f190ef782bd9deff3e42", "6d8c2810d1e870fbaa1f0a0937126cca837a15f9260e27060c331d70a3c0bc84"},
        {65536, "bf718b6f653bebc184e1479f1935b8da974d701b893afcf49e701f3e2f9f9c5a", "b7b4edb36ba92e267d322d56d9aebc22e7fa96ff52e3c12adc07f07a43cbc616"},
        {65537, "008ffc88d3c96a9f307524eb361e47c5222a887fc45fa0c1fb8d429c5c23b430", "eeb7c7c5373894ea2c1547cfd3ccb15d5a0b2d619da852e5c79df792dcc9e435"}
    };
    for (const auto &vector : extendedVectors) {
        const size_t prefixSize = vector.plaintextLength < 65536 ? 2 : 6;
        const size_t paddedSize = prefixSize + nostr::Nip44::paddedLength(vector.plaintextLength);
        const size_t rawSize = 1 + 32 + paddedSize + 32;
        const size_t encodedCapacity = ((rawSize + 2) / 3) * 4 + 1;
        std::vector<uint8_t> work(paddedSize, static_cast<uint8_t>('a'));
        std::vector<char> payload(encodedCapacity, 0);
        size_t encodedSize = 0;
        CHECK(nostr::Nip44::encryptWithConversationKey(
            work.data(), vector.plaintextLength, work.size(), extendedConversationKey, extendedNonce,
            payload.data(), payload.size(), encodedSize));
        CHECK(encodedSize + 1 == payload.size());
        uint8_t digest[32] = {};
        CHECK(nostr::Utils::sha256(reinterpret_cast<const uint8_t *>(payload.data()), encodedSize, digest) == 32);
        CHECK(nostr::Utils::toHex(digest, sizeof(digest)) == vector.payloadSha256);

        work.clear();
        work.shrink_to_fit();
        std::vector<uint8_t> decrypted(rawSize, 0);
        size_t decryptedSize = 0;
        CHECK(nostr::Nip44::decryptWithConversationKey(
            payload.data(), encodedSize, extendedConversationKey, decrypted.data(), decrypted.size(), decryptedSize));
        CHECK(decryptedSize == vector.plaintextLength);
        CHECK(nostr::Utils::sha256(decrypted.data(), decryptedSize, digest) == 32);
        CHECK(nostr::Utils::toHex(digest, sizeof(digest)) == vector.plaintextSha256);
    }

    String userPrivateKey = "0000000000000000000000000000000000000000000000000000000000000001";
    String servicePrivateKey = "0000000000000000000000000000000000000000000000000000000000000002";
    String userPublicKey = nostr::Utils::getPublicKey(userPrivateKey);
    String servicePublicKey = nostr::Utils::getPublicKey(servicePrivateKey);
    nostr::Nip44 nip44;
    String randomEncrypted = nip44.encrypt(userPrivateKey, servicePublicKey, "random NIP-44 message");
    CHECK(randomEncrypted.length() > 0);
    CHECK(nip44.decrypt(servicePrivateKey, userPublicKey, randomEncrypted) == "random NIP-44 message");

    String invalidVersion = payload1;
    invalidVersion.setCharAt(0, 'A');
    invalidVersion.setCharAt(1, 'A');
    CHECK(nostr::Nip44::decryptWithConversationKey(invalidVersion, conversationKey1) == "");
    CHECK(nostr::Nip44::decryptWithConversationKey("Ag==", conversationKey1) == "");
    CHECK(nostr::Nip44::decryptWithConversationKey("!!!!", conversationKey1) == "");
    CHECK(nostr::Nip44::decryptWithConversationKey("#" + payload1, conversationKey1) == "");
    String invalidMac = payload1;
    invalidMac.setCharAt(invalidMac.length() - 1, invalidMac.charAt(invalidMac.length() - 1) == 'A' ? 'B' : 'A');
    CHECK(nostr::Nip44::decryptWithConversationKey(invalidMac, conversationKey1) == "");
    CHECK(nostr::Nip44::encryptWithConversationKey("", conversationKey1, nonce1) == "");
    CHECK(nostr::Nip44::encryptWithConversationKey("x", "bad", nonce1) == "");
    return true;
}

} // namespace unit_test
