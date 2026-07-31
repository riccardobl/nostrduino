#ifndef NOSTR_LIMITS_H
#define NOSTR_LIMITS_H

#include <stddef.h>
#include <stdint.h>

namespace nostr {

struct NostrLimits {
    size_t maxWebSocketMessageBytes;
    size_t maxHttpResponseBytes;
    size_t maxUrlBytes;
    size_t maxSubscriptionIdBytes;
    size_t maxRelayMessageBytes;
    size_t maxEventContentBytes;
    size_t maxTagCount;
    size_t maxTagElements;
    size_t maxTagElementBytes;
    size_t maxNip04PlaintextBytes;
    size_t maxNip04CiphertextBytes;

    NostrLimits()
        : maxWebSocketMessageBytes(64 * 1024), maxHttpResponseBytes(64 * 1024),
          maxUrlBytes(2048), maxSubscriptionIdBytes(64), maxRelayMessageBytes(4096),
          maxEventContentBytes(32 * 1024), maxTagCount(256), maxTagElements(64),
          maxTagElementBytes(4096), maxNip04PlaintextBytes(32 * 1024),
          maxNip04CiphertextBytes(64 * 1024) {}
};

}

#endif
