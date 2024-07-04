#ifndef NIP47_H
#define NIP47_H
#include "NostrString.h"
namespace nostr {
    class Nip47 {
       public:
        bool payInvoice(NostrString invoice, unsigned long amount = -1);
        bool multiPayInvoice(NostrString invoices[][3]);
        bool payKeySend(NostrString pubkey, unsigned long amount,
                        NostrString preimage = "",
                        NostrString tlvRecords[][2] = {}, int size = 0);
        bool multiPayKeySend(NostrString response);
        bool makeInvoice(unsigned long amount, NostrString description = "",
                         NostrString descriptionHash = "", unsigned long expiry = 0);
        bool lookUpInvoice(NostrString response);
        bool listTransactions(unsigned long from = 0, unsigned long until = 0,
                              int limit = 0, int offset = 0,
                              bool unpaid = false, NostrString type = "");
        unsigned long int getBalance(NostrString response);
        bool getInfo();
    };
}  // namespace nostr

#endif  // NIP47_H