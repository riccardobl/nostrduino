#include "crypto/UBitcoinBackend.h"
#include <cassert>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

namespace {
int hexValue(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

std::vector<uint8_t> decodeHex(const std::string &hex) {
    assert(hex.size() % 2 == 0);
    std::vector<uint8_t> result(hex.size() / 2);
    for (size_t i = 0; i < result.size(); ++i) {
        const int high = hexValue(hex[i * 2]);
        const int low = hexValue(hex[i * 2 + 1]);
        assert(high >= 0 && low >= 0);
        result[i] = static_cast<uint8_t>((high << 4) | low);
    }
    return result;
}

std::vector<std::string> splitCsv(const std::string &line) {
    std::vector<std::string> fields;
    std::stringstream stream(line);
    std::string field;
    while (std::getline(stream, field, ',')) fields.push_back(field);
    while (fields.size() < 8) fields.emplace_back();
    return fields;
}
}

int main(int argc, char **argv) {
    using nostr::crypto::UBitcoinBackend;
    bool entropyAvailable = true;
    uint8_t entropyByte = 0;
    assert(UBitcoinBackend::initialize([&](uint8_t *output, size_t size) {
        if (!entropyAvailable || (output == nullptr && size != 0)) return false;
        for (size_t i = 0; i < size; ++i) output[i] = entropyByte++;
        return true;
    }));
    assert(argc == 2);
    std::ifstream vectors(argv[1]);
    assert(vectors.good());

    std::string line;
    assert(static_cast<bool>(std::getline(vectors, line)));
    size_t vectorCount = 0;
    while (std::getline(vectors, line)) {
        const std::vector<std::string> fields = splitCsv(line);
        assert(fields.size() >= 8);
        const std::vector<uint8_t> publicKey = decodeHex(fields[2]);
        const std::vector<uint8_t> message = decodeHex(fields[4]);
        const std::vector<uint8_t> signature = decodeHex(fields[5]);
        assert(publicKey.size() == 32 && signature.size() == 64);
        const bool expected = fields[6] == "TRUE";
        assert(UBitcoinBackend::verify(publicKey.data(), message.data(), message.size(), signature.data()) == expected);

        if (!fields[1].empty()) {
            const std::vector<uint8_t> secret = decodeHex(fields[1]);
            const std::vector<uint8_t> auxiliary = decodeHex(fields[3]);
            uint8_t derived[32] = {};
            uint8_t generated[64] = {};
            assert(secret.size() == 32 && auxiliary.size() == 32);
            assert(UBitcoinBackend::publicKey(secret.data(), derived));
            assert(std::memcmp(derived, publicKey.data(), sizeof(derived)) == 0);
            assert(UBitcoinBackend::sign(secret.data(), message.data(), message.size(), auxiliary.data(), generated));
            assert(std::memcmp(generated, signature.data(), sizeof(generated)) == 0);
        }
        ++vectorCount;
    }
    assert(vectorCount == 19);

    uint8_t zero[32] = {};
    uint8_t output[32] = {};
    const std::vector<uint8_t> curveOrder = decodeHex("FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEBAAEDCE6AF48A03BBFD25E8CD0364141");
    assert(!UBitcoinBackend::publicKey(zero, output));
    assert(!UBitcoinBackend::publicKey(curveOrder.data(), output));

    const std::vector<uint8_t> one = decodeHex("0000000000000000000000000000000000000000000000000000000000000001");
    const std::vector<uint8_t> two = decodeHex("0000000000000000000000000000000000000000000000000000000000000002");
    const std::vector<uint8_t> generator = decodeHex("79BE667EF9DCBBAC55A06295CE870B07029BFCDB2DCE28D959F2815B16F81798");
    const std::vector<uint8_t> twiceGenerator = decodeHex("C6047F9441ED7D6D3045406E95C07CD85C778E4B8CEF3CA7ABAC09B95C709EE5");
    uint8_t sharedA[32] = {};
    uint8_t sharedB[32] = {};
    assert(UBitcoinBackend::ecdhRawX(one.data(), twiceGenerator.data(), sharedA));
    assert(UBitcoinBackend::ecdhRawX(two.data(), generator.data(), sharedB));
    assert(std::memcmp(sharedA, sharedB, sizeof(sharedA)) == 0);

    entropyAvailable = false;
    std::memset(output, 0xa5, sizeof(output));
    assert(!UBitcoinBackend::publicKey(one.data(), output));
    for (uint8_t byte : output) assert(byte == 0);
    UBitcoinBackend::shutdown();
    assert(!UBitcoinBackend::isInitialized());
    return 0;
}
