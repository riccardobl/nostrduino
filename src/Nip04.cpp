#include "Nip04.h"

using namespace nostr;

NostrString Nip04::decrypt(NostrString &privateKeyHex, NostrString &senderPubKeyHex, NostrString content) {
    int ivParamIndex = NostrString_indexOf(content, "?iv=");
    if (ivParamIndex == -1) return "";

    NostrString encryptedMessage = NostrString_substring(content, 0, ivParamIndex);

    NostrString encryptedMessageHex = Utils::base64ToHex(encryptedMessage);
    int encryptedMessageSize = NostrString_length(encryptedMessageHex) / 2;
    if (!encryptedMessageSize) return NostrString(); // nothing to do (can happen if base64ToHex returns "" because out of memory)
    if (encryptedMessageSize % 16 != 0) return NostrString();

    NostrString iv = NostrString_substring(content, ivParamIndex + 4);
    NostrString ivHex = Utils::base64ToHex(iv);
    if (NostrString_length(ivHex) / 2 != 16) return NostrString();
    int ivSize = 16;
    byte ivBin[ivSize];
    if (!Utils::decodeHexExact(ivHex, ivBin, sizeof(ivBin))) return "";

    uint8_t sharedPointX[32];
    if (!Utils::ecdh(privateKeyHex, senderPubKeyHex, sharedPointX)) return "";
    NostrString sharedPointXHex = Utils::toHex(sharedPointX, 32);

    return decryptData(sharedPointX, ivBin, encryptedMessageHex);
}

NostrString Nip04::encrypt(NostrString &privateKeyHex, NostrString &recipientPubKeyHex, NostrString content) {
    uint8_t sharedPointX[32];
    if (!Utils::ecdh(privateKeyHex, recipientPubKeyHex, sharedPointX)) return "";
    NostrString sharedPointXHex = Utils::toHex(sharedPointX, 32);
    uint8_t iv[16];
    for (size_t i = 0; i < sizeof(iv); i++) {
        iv[i] = (uint8_t)Utils::randomInt(0, 255);
    }
    NostrString ivHex = Utils::toHex(iv, sizeof(iv));
    NostrString ivBase64 = Utils::hexToBase64(ivHex);
    NostrString encryptedMessageHex = encryptData(sharedPointX, iv, content);
    NostrString encryptedMessageBase64 = Utils::hexToBase64(encryptedMessageHex);
    encryptedMessageBase64 += "?iv=" + ivBase64;
    return encryptedMessageBase64;
}

NostrString Nip04::encryptData(byte key[32], byte iv[16], NostrString msg) {
    // message has to be padded at the end so it is a multiple of 16
    int padding_diff = NostrString_length(msg) % 16 == 0 ? 16 : 16 - (NostrString_length(msg) % 16);

    int byteSize = NostrString_length(msg) + padding_diff;
    byte messageBin[byteSize];
    stringToByteArray(msg, padding_diff, messageBin);

    AES_ctx ctx;
    AES_init_ctx_iv(&ctx, key, iv);

    AES_CBC_encrypt_buffer(&ctx, messageBin, sizeof(messageBin));

    return Utils::toHex(messageBin, sizeof(messageBin));
}

void Nip04::stringToByteArray(const NostrString &input, int padding_diff, byte *output) {
    int inputSize = NostrString_length(input);
    for (int i = 0; i < inputSize; i++) {
        output[i] = input[i];
    }
    // pad between 1 and 16 bytes
    for (int j = 0; j < padding_diff; j++) {
        output[inputSize + j] = padding_diff;
    }
}

NostrString Nip04::decryptData(byte key[32], byte iv[16], NostrString messageHex) {
    int byteSize = NostrString_length(messageHex) / 2;
    if (byteSize == 0 || byteSize % 16 != 0) return NostrString();
    byte* messageBin = (byte*)malloc(byteSize);
    if (!messageBin) return NostrString();
    if (!Utils::decodeHexExact(messageHex, messageBin, byteSize)) {
        free(messageBin);
        return "";
    }
    AES_ctx ctx;
    AES_init_ctx_iv(&ctx, key, iv);
    AES_CBC_decrypt_buffer(&ctx, messageBin, byteSize);
    int padding = messageBin[byteSize - 1];
    if (padding < 1 || padding > 16 || padding > byteSize) {
        free(messageBin);
        return NostrString();
    }
    for (int i = byteSize - padding; i < byteSize; i++) {
        if (messageBin[i] != padding) {
            free(messageBin);
            return NostrString();
        }
    }
    int resultSize = byteSize - padding;
    NostrString result;
    result.reserve(resultSize);
    for (int i = 0; i < resultSize; i++) {
        result += (char)messageBin[i];
    }
    free(messageBin);
    return result;
}
