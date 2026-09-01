#include "test_unit.h"

#include "nwc_test_support.h"

namespace unit_test {

bool testNwcConnection() {
    using namespace nwc_mock;

    const String serviceSecret =
        "0000000000000000000000000000000000000000000000000000000000000002";
    const String userSecret =
        "0000000000000000000000000000000000000000000000000000000000000001";
    const String servicePublicKey = nostr::Utils::getPublicKey(serviceSecret);
    const String prefix = "nostr+walletconnect://" + servicePublicKey;
    const String validUrl = prefix +
        "?relay=wss%3A%2F%2Frelay-one.example%2Fv1"
        "&relay=ws%3A%2F%2Frelay-two.example%3A8080"
        "&relay=wss%3A%2F%2Frelay-one.example%2Fv1"
        "&secret=" + userSecret + "&lud16=user%40example.com";

    nostr::NWCData data;
    CHECK(nostr::Nip47::parseNWC(validUrl, data));
    CHECK(data.pubkey == servicePublicKey);
    CHECK(data.secret == userSecret);
    CHECK(data.relays.size() == 2);
    CHECK(data.relay == "wss://relay-one.example/v1");
    CHECK(data.relays[1] == "ws://relay-two.example:8080");

    nostr::NWCData unchanged;
    unchanged.relay = "keep";
    unchanged.pubkey = "keep-pubkey";
    CHECK(!nostr::Nip47::parseNWC("x" + validUrl, unchanged));
    CHECK(unchanged.relay == "keep" && unchanged.pubkey == "keep-pubkey");
    CHECK(!nostr::Nip47::parseNWC(prefix + "?relay=wss%ZZrelay.example&secret=" + userSecret, unchanged));
    CHECK(!nostr::Nip47::parseNWC(prefix + "?relay=https%3A%2F%2Frelay.example&secret=" + userSecret, unchanged));
    CHECK(!nostr::Nip47::parseNWC(prefix + "?secret=" + userSecret, unchanged));
    CHECK(!nostr::Nip47::parseNWC(prefix + "?relay=wss%3A%2F%2Frelay.example&secret=" +
                                 userSecret + "&secret=" + userSecret, unchanged));
    CHECK(!nostr::Nip47::parseNWC(prefix + "?relay=wss%3A%2F%2Frelay.example&secret=" +
                                 repeatChar('0', 64), unchanged));
    CHECK(!nostr::Nip47::parseNWC("nostr+walletconnect://" + repeatChar('g', 64) +
                                 "?relay=wss%3A%2F%2Frelay.example&secret=" + userSecret, unchanged));

    MockTransport invalidTransport;
    nostr::NWC invalidClient(&invalidTransport, "not-an-nwc-url");
    CHECK(!invalidClient.isValid());
    CHECK(!invalidClient.isReady());
    CHECK(invalidTransport.connection == nullptr);
    CHECK(invalidTransport.connectedUrls.empty());
    int invalidErrors = 0;
    invalidClient.getBalance(nullptr, [&](String code, String) {
        if (code == "INVALID_CONNECTION") ++invalidErrors;
    });
    invalidClient.payInvoice("lnbc", -2, nullptr, [&](String code, String) {
        if (code == "INVALID_CONNECTION") ++invalidErrors;
    });
    invalidClient.loop();
    CHECK(invalidErrors == 2);
    CHECK(invalidTransport.connection == nullptr);

    MockTransport validTransport;
    nostr::NWC validClient(&validTransport, validUrl);
    CHECK(validClient.isValid());
    CHECK(validTransport.connectedUrls.size() == 2);
    CHECK(validTransport.connectedUrls[0] == "wss://relay-one.example/v1");
    CHECK(validTransport.connectedUrls[1] == "ws://relay-two.example:8080");
    validClient.close();

    return true;
}

} // namespace unit_test
