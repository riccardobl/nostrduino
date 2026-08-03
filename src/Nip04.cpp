#include "Nip04.h"
#include <limits>
#include <new>
#include <vector>

using namespace nostr;

#ifdef NOSTRDUINO_UNIT_TEST_HOOKS
extern "C" bool nostrduinoUnitTestFailNip04Allocation();
#endif

namespace {
void checkTestAllocationFailure() {
#ifdef NOSTRDUINO_UNIT_TEST_HOOKS
    if (nostrduinoUnitTestFailNip04Allocation()) throw std::bad_alloc();
#endif
}

void wipe(void *memory, size_t size) {
    if (memory == nullptr) return;
    volatile uint8_t *p = static_cast<volatile uint8_t *>(memory);
    while (size-- > 0) *p++ = 0;
}

bool base64Capacity(size_t encodedSize, size_t maxDecodedSize, size_t &capacity) {
    if (encodedSize == 0 || encodedSize % 4 != 0) return false;
    const size_t groups = encodedSize / 4;
    if (groups > (SIZE_MAX / 3)) return false;
    capacity = groups * 3;
    if (capacity > maxDecodedSize) capacity = maxDecodedSize;
    return capacity != 0;
}

bool decodeStandardBase64(const NostrString &encoded, size_t maxDecodedSize, std::vector<uint8_t> &decoded) {
    const long encodedLength = NostrString_length(encoded);
    if (encodedLength <= 0) return false;
    size_t capacity = 0;
    if (!base64Capacity(static_cast<size_t>(encodedLength), maxDecodedSize, capacity)) return false;
    try {
        checkTestAllocationFailure();
        decoded.assign(capacity, 0);
        const size_t decodedSize = Utils::fromBase64(encoded.c_str(), static_cast<size_t>(encodedLength),
                                                     decoded.data(), decoded.size(), BASE64_STANDARD);
        if (decodedSize == 0 || decodedSize > maxDecodedSize) {
            wipe(decoded.data(), decoded.size());
            decoded.clear();
            return false;
        }
        decoded.resize(decodedSize);
        return true;
    } catch (...) {
        if (!decoded.empty()) wipe(decoded.data(), decoded.size());
        decoded.clear();
        return false;
    }
}

bool encodedCapacity(size_t byteSize, size_t &capacity) {
    if (byteSize > SIZE_MAX - 2) return false;
    const size_t groups = (byteSize + 2) / 3;
    if (groups > (SIZE_MAX - 1) / 4) return false;
    capacity = groups * 4;
    return true;
}

NostrString encodeStandardBase64(const uint8_t *bytes, size_t byteSize) {
    size_t capacity = 0;
    if (!encodedCapacity(byteSize, capacity)) return "";
    try {
        checkTestAllocationFailure();
        std::vector<char> encoded(capacity + 1, 0);
        const size_t encodedSize = Utils::toBase64(bytes, byteSize, encoded.data(), encoded.size(), BASE64_STANDARD);
        if (encodedSize == 0 && byteSize != 0) return "";
        NostrString result = NostrString_fromBytes(encoded.data(), encodedSize);
        wipe(encoded.data(), encoded.size());
        return result;
    } catch (...) {
        return "";
    }
}
}

NostrString Nip04::decrypt(NostrString &privateKeyHex, NostrString &senderPubKeyHex, NostrString content) {
    const long contentLength = NostrString_length(content);
    size_t maxEncodedCiphertext = 0;
    if (contentLength <= 0 || !encodedCapacity(limits.maxNip04CiphertextBytes, maxEncodedCiphertext) ||
        maxEncodedCiphertext > SIZE_MAX - 28 ||
        static_cast<size_t>(contentLength) > maxEncodedCiphertext + 28) return "";

    const int ivParamIndex = NostrString_indexOf(content, "?iv=");
    if (ivParamIndex <= 0 || NostrString_indexOf(NostrString_substring(content, ivParamIndex + 1), "?iv=") != -1) return "";

    const NostrString encryptedMessage = NostrString_substring(content, 0, ivParamIndex);
    const NostrString ivBase64 = NostrString_substring(content, ivParamIndex + 4);
    std::vector<uint8_t> ciphertext;
    std::vector<uint8_t> iv;
    if (NostrString_length(ivBase64) != 24 ||
        !decodeStandardBase64(encryptedMessage, limits.maxNip04CiphertextBytes, ciphertext) ||
        !decodeStandardBase64(ivBase64, sizeof(uint8_t) * 16, iv) || iv.size() != 16 ||
        ciphertext.empty() || ciphertext.size() % 16 != 0) {
        if (!ciphertext.empty()) wipe(ciphertext.data(), ciphertext.size());
        if (!iv.empty()) wipe(iv.data(), iv.size());
        return "";
    }

    uint8_t sharedPointX[32] = {};
    const bool shared = Utils::ecdh(privateKeyHex, senderPubKeyHex, sharedPointX);
    NostrString result = shared ? decryptData(sharedPointX, iv.data(), ciphertext.data(), ciphertext.size()) : "";
    wipe(sharedPointX, sizeof(sharedPointX));
    wipe(ciphertext.data(), ciphertext.size());
    wipe(iv.data(), iv.size());
    return result;
}

NostrString Nip04::encrypt(NostrString &privateKeyHex, NostrString &recipientPubKeyHex, NostrString content) {
    const long contentLength = NostrString_length(content);
    if (contentLength < 0 || static_cast<size_t>(contentLength) > limits.maxNip04PlaintextBytes) return "";

    uint8_t sharedPointX[32] = {};
    uint8_t iv[16] = {};
    if (!Utils::ecdh(privateKeyHex, recipientPubKeyHex, sharedPointX) || !Utils::fillRandom(iv, sizeof(iv))) {
        wipe(sharedPointX, sizeof(sharedPointX));
        wipe(iv, sizeof(iv));
        return "";
    }
    NostrString encryptedMessageBase64 = encryptData(sharedPointX, iv, content.c_str(), static_cast<size_t>(contentLength));
    const NostrString ivBase64 = encodeStandardBase64(iv, sizeof(iv));
    wipe(sharedPointX, sizeof(sharedPointX));
    wipe(iv, sizeof(iv));
    if (NostrString_length(encryptedMessageBase64) == 0 || NostrString_length(ivBase64) != 24) return "";
    encryptedMessageBase64 += "?iv=";
    encryptedMessageBase64 += ivBase64;
    return encryptedMessageBase64;
}

NostrString Nip04::encryptData(const uint8_t key[32], const uint8_t iv[16], const char *message, size_t messageSize) {
    if (key == nullptr || iv == nullptr || message == nullptr || messageSize > limits.maxNip04PlaintextBytes) return "";
    const size_t padding = AES_BLOCKLEN - (messageSize % AES_BLOCKLEN);
    if (messageSize > SIZE_MAX - padding) return "";
    const size_t ciphertextSize = messageSize + padding;
    if (ciphertextSize > limits.maxNip04CiphertextBytes) return "";
    std::vector<uint8_t> buffer;
    AES_ctx ctx = {};
    try {
        checkTestAllocationFailure();
        buffer.assign(ciphertextSize, static_cast<uint8_t>(padding));
        for (size_t i = 0; i < messageSize; ++i) buffer[i] = static_cast<uint8_t>(message[i]);
        AES_init_ctx_iv(&ctx, key, iv);
        AES_CBC_encrypt_buffer(&ctx, buffer.data(), buffer.size());
        NostrString result = encodeStandardBase64(buffer.data(), buffer.size());
        wipe(&ctx, sizeof(ctx));
        wipe(buffer.data(), buffer.size());
        return result;
    } catch (...) {
        wipe(&ctx, sizeof(ctx));
        if (!buffer.empty()) wipe(buffer.data(), buffer.size());
        return "";
    }
}

NostrString Nip04::decryptData(const uint8_t key[32], const uint8_t iv[16], const uint8_t *ciphertext, size_t ciphertextSize) {
    if (key == nullptr || iv == nullptr || ciphertext == nullptr || ciphertextSize == 0 ||
        ciphertextSize % AES_BLOCKLEN != 0 || ciphertextSize > limits.maxNip04CiphertextBytes) return "";
    std::vector<uint8_t> buffer;
    try {
        checkTestAllocationFailure();
        buffer.assign(ciphertext, ciphertext + ciphertextSize);
        AES_ctx ctx;
        AES_init_ctx_iv(&ctx, key, iv);
        AES_CBC_decrypt_buffer(&ctx, buffer.data(), buffer.size());
        wipe(&ctx, sizeof(ctx));

        const uint8_t padding = buffer.back();
        if (padding == 0 || padding > AES_BLOCKLEN || padding > buffer.size()) {
            wipe(buffer.data(), buffer.size());
            return "";
        }
        uint8_t mismatch = 0;
        for (size_t i = buffer.size() - padding; i < buffer.size(); ++i) mismatch |= buffer[i] ^ padding;
        if (mismatch != 0 || buffer.size() - padding > limits.maxNip04PlaintextBytes) {
            wipe(buffer.data(), buffer.size());
            return "";
        }
        NostrString result = NostrString_fromBytes(reinterpret_cast<const char *>(buffer.data()), buffer.size() - padding);
        wipe(buffer.data(), buffer.size());
        return result;
    } catch (...) {
        if (!buffer.empty()) wipe(buffer.data(), buffer.size());
        return "";
    }
}
