#ifndef _NIP04_H
#define _NIP04_H
#include <Arduino.h>
#include <aes.h>
#include <vector>
#include "NostrString.h"
#include "Utils.h"
namespace nostr {
class Nip04 {
  public:
    NostrString decrypt(NostrString &privateKeyHex, NostrString &senderPubKeyHex, NostrString content);
    NostrString encrypt(NostrString &privateKeyHex, NostrString &recipientPubKeyHex, NostrString content);
  private:
    NostrString encryptData(byte key[32], byte iv[16], NostrString msg);
    void stringToByteArray(const char *input, int padding_diff, byte *output);
    NostrString decryptData(byte key[32], byte iv[16], NostrString messageHex);
};
} 
#endif 