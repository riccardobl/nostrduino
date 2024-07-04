#ifndef NIP04_H
#define NIP04_H
#include <aes.h>

#include "Hash.h"
#include "NostrString.h"
#include "Utils.h"

 
namespace nostr {
    class Nip04 {
       public:
        static NostrString decrypt(NostrString& privateKeyHex,
                                   NostrString& senderPubKeyHex,
                                   NostrString content);
        static NostrString encrypt(NostrString &privateKeyHex,
                                   NostrString& recipientPubKeyHex,
                                   NostrString content);

       private:
        static  NostrString encryptData(byte key[32], byte iv[16],
                                       NostrString msg) ;

        static  void stringToByteArray(const char *input, int padding_diff,
                               byte *output) ;

        static  NostrString decryptData(byte key[32], byte iv[16],
                                       NostrString messageHex) ;
    };
}  // namespace nostr
#endif  // NIP47_H