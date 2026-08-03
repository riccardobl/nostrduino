#include "test_unit.h"

#include "NostrLimits.h"
#include "NostrString.h"
#include "NostrUrl.h"

namespace unit_test {

bool testTransport() {
    char payload[] = {'a', '\0', 'b', 'c'};
    String message;
    CHECK(NostrString_appendBytes(message, payload, sizeof(payload), 4));
    CHECK(message.length() == sizeof(payload));
    CHECK(message.charAt(1) == '\0');
    CHECK(!NostrString_appendBytes(message, payload, 1, 4));
    CHECK(NostrString_substring(String("abcdef"), 1, 3) == "bc");
    String urlEncoded = "a b";
    urlEncoded += static_cast<char>(1);
    CHECK(NostrString_urlEncode(urlEncoded) == "a%20b%01");
    String urlDecoded = "a%20b%01";
    String expectedDecoded = "a b";
    expectedDecoded += static_cast<char>(1);
    CHECK(NostrString_urlDecode(urlDecoded) == expectedDecoded);
    CHECK(NostrString_urlDecode("%") == "");
    CHECK(NostrString_fromUInt(18446744073709551615ULL) == "18446744073709551615");

    nostr::ParsedUrl parsed;
    CHECK(nostr::parseWebSocketUrl("wss://relay.example.com", parsed));
    CHECK(parsed.secure && parsed.host == "relay.example.com" && parsed.port == 443 && parsed.path == "/");
    CHECK(nostr::parseWebSocketUrl("ws://localhost:8080/path?x=1", parsed));
    CHECK(!parsed.secure && parsed.host == "localhost" && parsed.port == 8080 && parsed.path == "/path?x=1");
    CHECK(nostr::parseWebSocketUrl("wss://relay.example.com?x=1", parsed) && parsed.path == "/?x=1");
    CHECK(nostr::parseHttpsUrl("https://example.com/a?b=c", parsed));
    const char *invalid[] = {"relay.example.com", "ftp://example.com", "ws://", "ws://host:0",
                             "ws://host:65536", "ws://user@host/", "ws://host/#fragment",
                             "ws://bad_host/", "https://host path/"};
    for (const char *value : invalid) CHECK(!nostr::parseWebSocketUrl(value, parsed));
    CHECK(!nostr::parseHttpsUrl("http://example.com", parsed));

    nostr::NostrLimits limits;
    CHECK(limits.maxWebSocketMessageBytes == 64 * 1024);
    CHECK(limits.maxHttpResponseBytes == 64 * 1024);
    CHECK(limits.maxSubscriptionIdBytes == 64);
    CHECK(limits.maxRelayMessageBytes == 4096);
    CHECK(limits.maxNip04PlaintextBytes < limits.maxNip04CiphertextBytes);
    return true;
}

} // namespace unit_test
