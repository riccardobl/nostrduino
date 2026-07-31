#ifndef _NIP04_H
#define _NIP04_H

#include "NostrLimits.h"
#include "NostrString.h"
#include "NostrUtils.h"
#include <Arduino.h>
#include <NostrAES.h>

namespace nostr {
class Nip04 {
  public:
    explicit Nip04(const NostrLimits &limits = NostrLimits()) : limits(limits) {}
    NostrString decrypt(NostrString &privateKeyHex, NostrString &senderPubKeyHex, NostrString content);
    NostrString encrypt(NostrString &privateKeyHex, NostrString &recipientPubKeyHex, NostrString content);

  private:
    NostrString encryptData(const uint8_t key[32], const uint8_t iv[16], const char *message, size_t messageSize);
    NostrString decryptData(const uint8_t key[32], const uint8_t iv[16], const uint8_t *ciphertext, size_t ciphertextSize);
    NostrLimits limits;
};
}

#endif
