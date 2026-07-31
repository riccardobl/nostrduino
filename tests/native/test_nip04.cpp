#include "Nip04.h"
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <new>

namespace {
const char alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
uint8_t randomByte = 0x10;
bool randomAvailable = true;
bool ecdhAvailable = true;
bool failAllocation = false;

bool decodeValue(const char *encoded, size_t encodedSize, uint8_t *output, size_t &outputSize) {
    outputSize = 0;
    for (size_t i = 0; i < encodedSize; i += 4) {
        uint32_t value = 0;
        for (size_t j = 0; j < 4; ++j) {
            const char c = encoded[i + j];
            value <<= 6;
            if (c != '=') {
                const char *found = std::strchr(alphabet, c);
                if (found == nullptr) return false;
                value |= static_cast<uint32_t>(found - alphabet);
            }
        }
        output[outputSize++] = static_cast<uint8_t>(value >> 16);
        if (encoded[i + 2] != '=') output[outputSize++] = static_cast<uint8_t>(value >> 8);
        if (encoded[i + 3] != '=') output[outputSize++] = static_cast<uint8_t>(value);
    }
    return true;
}
}

void *operator new(size_t size) {
    if (failAllocation) throw std::bad_alloc();
    if (void *memory = std::malloc(size)) return memory;
    throw std::bad_alloc();
}
void *operator new[](size_t size) { return ::operator new(size); }
void operator delete(void *memory) noexcept { std::free(memory); }
void operator delete[](void *memory) noexcept { std::free(memory); }
void operator delete(void *memory, size_t) noexcept { std::free(memory); }
void operator delete[](void *memory, size_t) noexcept { std::free(memory); }

namespace {
std::string encode(const uint8_t *bytes, size_t size) {
    std::string result(((size + 2) / 3) * 4, '\0');
    const size_t encoded = nostr::Utils::toBase64(bytes, size, result.data(), result.size(), BASE64_STANDARD);
    result.resize(encoded);
    return result;
}

std::string encryptedBlock(const uint8_t plaintext[16]) {
    uint8_t key[32];
    uint8_t iv[16] = {};
    uint8_t block[16];
    for (size_t i = 0; i < sizeof(key); ++i) key[i] = static_cast<uint8_t>(i + 1);
    std::memcpy(block, plaintext, sizeof(block));
    AES_ctx context = {};
    AES_init_ctx_iv(&context, key, iv);
    AES_CBC_encrypt_buffer(&context, block, sizeof(block));
    return encode(block, sizeof(block)) + "?iv=" + encode(iv, sizeof(iv));
}
}

namespace nostr {
std::function<void(const NostrString &)> Utils::logger = nullptr;
unsigned long long Utils::subs = 0;
std::function<bool(uint8_t *, size_t)> Utils::randomBytesProvider = nullptr;
std::function<unsigned long long()> Utils::timeSecondsProvider = nullptr;

bool Utils::fillRandom(uint8_t *output, size_t outputSize) {
    if (!randomAvailable || (output == nullptr && outputSize != 0)) return false;
    for (size_t i = 0; i < outputSize; ++i) output[i] = randomByte++;
    return true;
}
bool Utils::ecdh(NostrString, NostrString, uint8_t *out) {
    if (!ecdhAvailable || out == nullptr) return false;
    for (size_t i = 0; i < 32; ++i) out[i] = static_cast<uint8_t>(i + 1);
    return true;
}
size_t Utils::toBase64(const uint8_t *array, size_t arraySize, char *output, size_t outputSize, uint8_t) {
    const size_t required = ((arraySize + 2) / 3) * 4;
    if (outputSize < required) return 0;
    size_t outputIndex = 0;
    for (size_t i = 0; i < arraySize; i += 3) {
        const size_t remaining = arraySize - i;
        const uint32_t value = (static_cast<uint32_t>(array[i]) << 16) |
                               (remaining > 1 ? static_cast<uint32_t>(array[i + 1]) << 8 : 0) |
                               (remaining > 2 ? array[i + 2] : 0);
        output[outputIndex++] = alphabet[(value >> 18) & 0x3f];
        output[outputIndex++] = alphabet[(value >> 12) & 0x3f];
        output[outputIndex++] = remaining > 1 ? alphabet[(value >> 6) & 0x3f] : '=';
        output[outputIndex++] = remaining > 2 ? alphabet[value & 0x3f] : '=';
    }
    return outputIndex;
}
size_t Utils::fromBase64(const char *encoded, size_t encodedSize, uint8_t *output, size_t outputSize, uint8_t) {
    if (encodedSize == 0 || encodedSize % 4 != 0) return 0;
    size_t decodedSize = (encodedSize / 4) * 3;
    if (encoded[encodedSize - 1] == '=') --decodedSize;
    if (encoded[encodedSize - 2] == '=') --decodedSize;
    if (outputSize < decodedSize) return 0;
    if (!decodeValue(encoded, encodedSize, output, decodedSize)) return 0;
    return decodedSize;
}
}

int main() {
    nostr::Nip04 nip04;
    NostrString privateKey = "private";
    NostrString publicKey = "public";
    const NostrString plaintext = "NIP-04 test with binary-safe framing";
    const NostrString encrypted = nip04.encrypt(privateKey, publicKey, plaintext);
    assert(encrypted.find("?iv=") != NostrString::npos);
    assert(nip04.decrypt(privateKey, publicKey, encrypted) == plaintext);
    assert(nip04.decrypt(privateKey, publicKey, encrypted + "?iv=AAAA") == "");
    assert(nip04.decrypt(privateKey, publicKey, encrypted.substr(0, encrypted.find("?iv=")) + "?iv=!!!!") == "");
    assert(nip04.decrypt(privateKey, publicKey, encrypted.substr(0, encrypted.find("?iv=")) + "?iv=AAAA") == "");
    assert(nip04.decrypt(privateKey, publicKey, "AAAA") == "");
    assert(nip04.decrypt(privateKey, publicKey, "AAAA?iv=AAAAAAAAAAAAAAAAAAAAAA==") == "");

    uint8_t paddingZero[16] = {};
    assert(nip04.decrypt(privateKey, publicKey, encryptedBlock(paddingZero)) == "");
    uint8_t paddingTooLarge[16] = {};
    paddingTooLarge[15] = 17;
    assert(nip04.decrypt(privateKey, publicKey, encryptedBlock(paddingTooLarge)) == "");
    uint8_t paddingMismatch[16] = {};
    paddingMismatch[14] = 1;
    paddingMismatch[15] = 2;
    assert(nip04.decrypt(privateKey, publicKey, encryptedBlock(paddingMismatch)) == "");

    nostr::NostrLimits smallLimits;
    smallLimits.maxNip04PlaintextBytes = 15;
    smallLimits.maxNip04CiphertextBytes = 16;
    nostr::Nip04 bounded(smallLimits);
    const NostrString boundaryPlaintext(15, 'x');
    const NostrString boundaryEncrypted = bounded.encrypt(privateKey, publicKey, boundaryPlaintext);
    assert(boundaryEncrypted.size() == 52);
    assert(bounded.decrypt(privateKey, publicKey, boundaryEncrypted) == boundaryPlaintext);
    assert(bounded.encrypt(privateKey, publicKey, NostrString(16, 'x')) == "");
    assert(bounded.decrypt(privateKey, publicKey, encrypted) == "");

    randomAvailable = false;
    assert(nip04.encrypt(privateKey, publicKey, plaintext) == "");
    randomAvailable = true;
    ecdhAvailable = false;
    assert(nip04.encrypt(privateKey, publicKey, plaintext) == "");
    assert(nip04.decrypt(privateKey, publicKey, encrypted) == "");
    ecdhAvailable = true;
    failAllocation = true;
    assert(nip04.encrypt(privateKey, publicKey, "x") == "");
    failAllocation = false;
    return 0;
}
