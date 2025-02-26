#include "Nip04.h"

using namespace nostr;

NostrString Nip04::decrypt(NostrString &privateKeyHex, NostrString &senderPubKeyHex, NostrString content) {
    int ivParamIndex = NostrString_indexOf(content, "?iv=");
    if (ivParamIndex == -1) throw std::invalid_argument("Encrypted string does not contain ?iv= parameter.");

    NostrString encryptedMessage = NostrString_substring(content, 0, ivParamIndex);

    NostrString encryptedMessageHex = Utils::base64ToHex(encryptedMessage);
    int encryptedMessageSize = NostrString_length(encryptedMessageHex) / 2;
    if (!encryptedMessageSize) return NostrString(); // nothing to do (can happen if base64ToHex returns "" because out of memory)

    NostrString iv = NostrString_substring(content, ivParamIndex + 4);
    NostrString ivHex = Utils::base64ToHex(iv);
    int ivSize = 16;
    byte ivBin[ivSize];
    Utils::fromHex(ivHex, ivBin, ivSize);

    uint8_t sharedPointX[32];
    Utils::ecdh(privateKeyHex, senderPubKeyHex, sharedPointX);
    NostrString sharedPointXHex = Utils::toHex(sharedPointX, 32);

    NostrString message = decryptData(sharedPointX, ivBin, encryptedMessageHex);
    message = NostrString_trim(message);

    return message;
}

NostrString Nip04::encrypt(NostrString &privateKeyHex, NostrString &recipientPubKeyHex, NostrString content) {
    uint8_t sharedPointX[32];
    Utils::ecdh(privateKeyHex, recipientPubKeyHex, sharedPointX);
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
    stringToByteArray(NostrString_toChars(msg), padding_diff, messageBin);

    AES_ctx ctx;
    AES_init_ctx_iv(&ctx, key, iv);

    AES_CBC_encrypt_buffer(&ctx, messageBin, sizeof(messageBin));

    return Utils::toHex(messageBin, sizeof(messageBin));
}

void Nip04::stringToByteArray(const char *input, int padding_diff, byte *output) {
    int i = 0;
    // remove end-of-string char
    while (input[i] != '\0') {
        output[i] = input[i];
        i++;
    }
    // pad between 1 and 16 bytes
    for (int j = 0; j < padding_diff; j++) {
        output[i + j] = padding_diff;
    }
}

NostrString Nip04::decryptData(byte key[32], byte iv[16], NostrString messageHex) {
    int byteSize = NostrString_length(messageHex) / 2;
    byte* messageBin = (byte*)malloc(byteSize);
    if (!messageBin) return NostrString();
    Utils::fromHex(messageHex, messageBin, byteSize);
    AES_ctx ctx;
    AES_init_ctx_iv(&ctx, key, iv);
    AES_CBC_decrypt_buffer(&ctx, messageBin, byteSize);
    NostrString result = NostrString_substring(NostrString((char *)messageBin),0, byteSize);
    free(messageBin);
    return result;
}