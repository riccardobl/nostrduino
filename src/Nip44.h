#ifndef NOSTR_NIP44_H
#define NOSTR_NIP44_H

#include "NostrLimits.h"
#include "NostrString.h"

namespace nostr {

class Nip44 {
  public:
    explicit Nip44(const NostrLimits &limits = NostrLimits()) : limits(limits) {}

    NostrString encrypt(NostrString &privateKeyHex, NostrString &recipientPubKeyHex, NostrString content);
    NostrString decrypt(NostrString &privateKeyHex, NostrString &senderPubKeyHex, NostrString content);

    // Deterministic helpers are useful for protocol vectors and callers that
    // already manage a conversation key and nonce securely.
    static NostrString conversationKey(const NostrString &privateKeyHex, const NostrString &publicKeyHex);
    static NostrString encryptWithConversationKey(NostrString plaintext, const NostrString &conversationKeyHex,
                                                   const NostrString &nonceHex);
    static NostrString decryptWithConversationKey(const NostrString &payload, const NostrString &conversationKeyHex);

    // Binary buffer variants avoid Arduino String's 16-bit length limit. The
    // encryption buffer is consumed and overwritten with ciphertext.
    static bool encryptWithConversationKey(uint8_t *plaintextBuffer, size_t plaintextSize,
                                           size_t plaintextBufferCapacity, const uint8_t conversationKey[32],
                                           const uint8_t nonce[32], char *payloadBuffer, size_t payloadCapacity,
                                           size_t &payloadSize);
    static bool decryptWithConversationKey(const char *payload, size_t payloadSize,
                                           const uint8_t conversationKey[32], uint8_t *plaintextBuffer,
                                           size_t plaintextBufferCapacity, size_t &plaintextSize);
    static size_t paddedLength(size_t plaintextLength);

  private:
    NostrLimits limits;
};

} // namespace nostr

#endif
