#include "NostrLimits.h"
#include "NostrString.h"
#include "NostrUrl.h"
#include <cassert>

int main() {
    char payload[] = {'a', '\0', 'b', 'c'};
    std::string message;
    assert(NostrString_appendBytes(message, payload, sizeof(payload), 4));
    assert(message.size() == sizeof(payload));
    assert(message[1] == '\0');
    assert(!NostrString_appendBytes(message, payload, 1, 4));
    assert(NostrString_substring(std::string("abcdef"), 1, 3) == "bc");
    assert(NostrString_urlEncode(std::string("a b\x01")) == "a%20b%01");
    assert(NostrString_urlDecode(std::string("a%20b%01")) == std::string("a b\x01", 4));
    assert(NostrString_urlDecode(std::string("%")) == "");
    assert(NostrString_fromUInt(18446744073709551615ULL) == "18446744073709551615");

    nostr::ParsedUrl parsed;
    assert(nostr::parseWebSocketUrl("wss://relay.example.com", parsed));
    assert(parsed.secure && parsed.host == "relay.example.com" && parsed.port == 443 && parsed.path == "/");
    assert(nostr::parseWebSocketUrl("ws://localhost:8080/path?x=1", parsed));
    assert(!parsed.secure && parsed.host == "localhost" && parsed.port == 8080 && parsed.path == "/path?x=1");
    assert(nostr::parseWebSocketUrl("wss://relay.example.com?x=1", parsed) && parsed.path == "/?x=1");
    assert(nostr::parseHttpsUrl("https://example.com/a?b=c", parsed));
    for (const char *invalid : {"relay.example.com", "ftp://example.com", "ws://", "ws://host:0",
                                "ws://host:65536", "ws://user@host/", "ws://host/#fragment",
                                "ws://bad_host/", "https://host path/"}) {
        assert(!nostr::parseWebSocketUrl(invalid, parsed));
    }
    assert(!nostr::parseHttpsUrl("http://example.com", parsed));

    nostr::NostrLimits limits;
    assert(limits.maxWebSocketMessageBytes == 64 * 1024);
    assert(limits.maxHttpResponseBytes == 64 * 1024);
    assert(limits.maxSubscriptionIdBytes == 64);
    assert(limits.maxRelayMessageBytes == 4096);
    assert(limits.maxNip04PlaintextBytes < limits.maxNip04CiphertextBytes);
    return 0;
}
