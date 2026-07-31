#ifndef NOSTR_URL_H
#define NOSTR_URL_H

#include "NostrString.h"
#include <stdint.h>

namespace nostr {

struct ParsedUrl {
    NostrString host;
    NostrString path;
    uint16_t port = 0;
    bool secure = false;
};

bool parseWebSocketUrl(const NostrString &url, ParsedUrl &parsed);
bool parseHttpsUrl(const NostrString &url, ParsedUrl &parsed);

} // namespace nostr

#endif
