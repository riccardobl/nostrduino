#ifndef NOSTR_ENCODING_H
#define NOSTR_ENCODING_H

#include "NostrString.h"
#include <stddef.h>
#include <stdint.h>

namespace nostr {
bool decodeHexExact(const NostrString &input, uint8_t *output, size_t expectedBytes);
bool decodeHexExact(const NostrString &input, uint8_t *output, size_t outputCapacity, size_t expectedBytes);
}

#endif
