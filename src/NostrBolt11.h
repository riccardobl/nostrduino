#ifndef NOSTR_BOLT11_H
#define NOSTR_BOLT11_H

#include "NostrAmount.h"
#include "NostrString.h"

namespace nostr {

// Validates checksum, amount, network, required fields, expiry and signature.
// expectedCurrency is the BOLT11 currency prefix without "ln" (for example
// "bc" or "tb"); an empty value accepts any standard Bitcoin network.
bool validateBolt11Invoice(const NostrString &invoice, MilliSats expectedAmount,
                           const NostrString &expectedCurrency = "",
                           unsigned long long nowSeconds = 0);

} // namespace nostr

#endif
