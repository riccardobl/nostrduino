#ifndef NOSTR_ENCODING_H
#define NOSTR_ENCODING_H

#include "NostrString.h"
#include <stddef.h>
#include <stdint.h>

namespace nostr {
bool decodeHexExact(const NostrString &input, uint8_t *output, size_t expectedBytes);

bool validateBase64Encoding(const char *encoded, size_t encodedSize, bool noPadding,
                            bool urlSafe, size_t &decodedSize);
bool decodeHexExact(const NostrString &input, uint8_t *output, size_t outputCapacity, size_t expectedBytes);
}

#endif
