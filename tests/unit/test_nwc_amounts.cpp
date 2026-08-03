#include "test_unit.h"

#include "NostrAmount.h"
#include "NostrString.h"
#include "NostrUtils.h"
#include "esp32/ESP32Transport.h"
#include "nwc_test_support.h"
#include <limits>
#include <type_traits>

namespace unit_test {

bool testNwcAmounts() {
    using namespace nwc_mock;

    static_assert(sizeof(nostr::MilliSats) == 8, "MilliSats must remain 64-bit");
    static_assert(std::is_signed<nostr::MilliSats>::value, "MilliSats must remain signed");
    static_assert(std::numeric_limits<nostr::MilliSats>::max() >= INT64_MAX,
                  "MilliSats must represent INT64_MAX");
    static_assert(nostr::MILLISATS_UNSPECIFIED == -1, "the omission sentinel changed");
    static_assert(nostr::MAX_MILLISATS == INT64_C(100000000000000),
                  "the default amount guard changed");

    const struct {
        nostr::MilliSats value;
        const char *decimal;
    } conversionCases[] = {
        {INT64_MIN, "-9223372036854775808"},
        {INT64_C(-1), "-1"},
        {INT64_C(0), "0"},
        {INT64_C(4294967295), "4294967295"},
        {INT64_C(4294967296), "4294967296"},
        {INT64_C(5000000000), "5000000000"},
        {INT64_C(100000000000), "100000000000"},
        {INT64_MAX, "9223372036854775807"},
    };
    for (const auto &testCase : conversionCases) {
        CHECK(NostrString_fromMilliSats(testCase.value) == testCase.decimal);
    }

    Fixture fixture;
    int invalidCalls = 0;
    bool invalidCode = true;
    auto invalid = [&](String code, String) {
        invalidCode = invalidCode && code == "INVALID_AMOUNT";
        ++invalidCalls;
    };

    fixture.client.payInvoice("lnbc-invalid", -2, nullptr, invalid);
    fixture.client.payInvoice("lnbc-int64-min", INT64_MIN, nullptr, invalid);
    fixture.client.payKeySend(fixture.servicePublicKey, 0, "", {}, nullptr, invalid);
    fixture.client.payKeySend(fixture.servicePublicKey, -1, "", {}, nullptr, invalid);
    fixture.client.payKeySend(fixture.servicePublicKey, -2, "", {}, nullptr, invalid);
    fixture.client.multiPayInvoice({{"lnbc-invalid", -1}}, nullptr, invalid);
    fixture.client.multiPayInvoice({{"lnbc-valid", 1}, {"lnbc-invalid", -2}}, nullptr, invalid);
    fixture.client.multiPayKeySend({{fixture.servicePublicKey, 0, "", {}}}, nullptr, invalid);
    fixture.client.multiPayKeySend({{fixture.servicePublicKey, 1, "", {}},
                                    {fixture.servicePublicKey, -1, "", {}}}, nullptr, invalid);
    fixture.client.makeInvoice(0, "", "", 0, nullptr, invalid);
    fixture.client.makeInvoice(-1, "", "", 0, nullptr, invalid);
    fixture.client.makeInvoice(-2, "", "", 0, nullptr, invalid);
#if NOSTRDUINO_ENABLE_AMOUNT_GUARDS
    fixture.client.makeInvoice(nostr::MAX_MILLISATS + 1, "", "", 0, nullptr, invalid);
    fixture.client.makeInvoice(INT64_MAX, "", "", 0, nullptr, invalid);
    CHECK(invalidCalls == 14);
#else
    CHECK(invalidCalls == 12);
#endif
    CHECK(invalidCode);
    CHECK(fixture.requestCount() == 0);
    CHECK(fixture.negotiate());
    CHECK(fixture.requestCount() == 0);

    nostr::Nip47 lowLevel(nostr::Nip04(), fixture.servicePublicKey, fixture.userPrivateKey);
    JsonDocument request;
    auto decryptRequest = [&](nostr::SignedNostrEvent &event) {
        if (!event.isValid()) return false;
        String privateKey = fixture.servicePrivateKey;
        String publicKey = fixture.userPublicKey;
        String plaintext = nostr::Nip04().decrypt(privateKey, publicKey, event.getContent());
        request.clear();
        return plaintext.length() > 0 && nostr::Utils::jsonParse(&plaintext, &request);
    };

    nostr::SignedNostrEvent event = lowLevel.payInvoice("lnbc-omitted");
    CHECK(decryptRequest(event));
    CHECK(request["method"].as<String>() == "pay_invoice");
    CHECK(request["params"]["amount"].isNull());
    event = lowLevel.payInvoice("lnbc-literal-minus-one", -1);
    CHECK(decryptRequest(event));
    CHECK(request["params"]["amount"].isNull());
    event = lowLevel.payInvoice("lnbc-typed-sentinel", nostr::MILLISATS_UNSPECIFIED);
    CHECK(decryptRequest(event));
    CHECK(request["params"]["amount"].isNull());
    event = lowLevel.payInvoice("lnbc-zero", 0);
    CHECK(decryptRequest(event));
    CHECK(request["params"]["amount"].isNull());

    event = lowLevel.payInvoice("lnbc-uint32-max", INT64_C(4294967295));
    CHECK(decryptRequest(event));
    CHECK(request["params"]["amount"].is<int64_t>());
    CHECK(request["params"]["amount"].as<int64_t>() == INT64_C(4294967295));

    std::vector<nostr::Invoice> exactInvoices = {
        {"lnbc-1", INT64_C(1)},
        {"lnbc-1000", INT64_C(1000)},
        {"lnbc-2100", INT64_C(2100)},
        {"lnbc-uint32-max", INT64_C(4294967295)},
        {"lnbc-uint32-plus-one", INT64_C(4294967296)},
        {"lnbc-5-billion", INT64_C(5000000000)},
        {"lnbc-one-btc", INT64_C(100000000000)},
        {"lnbc-guard-maximum", nostr::MAX_MILLISATS},
    };
#if !NOSTRDUINO_ENABLE_AMOUNT_GUARDS
    exactInvoices.push_back({"lnbc-above-default-maximum", nostr::MAX_MILLISATS + 1});
    exactInvoices.push_back({"lnbc-int64-max", INT64_MAX});
#endif
    event = lowLevel.multiPayInvoice(exactInvoices);
    CHECK(decryptRequest(event));
    CHECK(request["method"].as<String>() == "multi_pay_invoice");
    JsonArray exactJsonInvoices = request["params"]["invoices"].as<JsonArray>();
    CHECK(exactJsonInvoices.size() == exactInvoices.size());
    for (size_t index = 0; index < exactInvoices.size(); ++index) {
        CHECK(exactJsonInvoices[index]["amount"].is<int64_t>());
        CHECK(exactJsonInvoices[index]["amount"].as<int64_t>() == exactInvoices[index].amount);
    }

    event = lowLevel.makeInvoice(nostr::MAX_MILLISATS);
    CHECK(decryptRequest(event));
    CHECK(request["method"].as<String>() == "make_invoice");
    CHECK(request["params"]["amount"].as<int64_t>() == nostr::MAX_MILLISATS);
    event = lowLevel.payKeySend(fixture.servicePublicKey, INT64_C(4294967296));
    CHECK(decryptRequest(event));
    CHECK(request["params"]["amount"].as<int64_t>() == INT64_C(4294967296));
    event = lowLevel.multiPayKeySend(
        {{fixture.servicePublicKey, INT64_C(4294967296), "", {}}});
    CHECK(decryptRequest(event));
    CHECK(request["params"]["key_sends"][0]["amount"].as<int64_t>() == INT64_C(4294967296));

    fixture.loop();
    CHECK(fixture.requestCount() == 0);

    CHECK(!lowLevel.payInvoice("lnbc-invalid", -2).isValid());
    CHECK(!lowLevel.payInvoice("lnbc-int64-min", INT64_MIN).isValid());
    CHECK(!lowLevel.makeInvoice(0).isValid());
    CHECK(!lowLevel.makeInvoice(-1).isValid());
    CHECK(!lowLevel.makeInvoice(-2).isValid());
    CHECK(!lowLevel.payKeySend(fixture.servicePublicKey, 0).isValid());
    CHECK(!lowLevel.payKeySend(fixture.servicePublicKey, -1).isValid());
    CHECK(!lowLevel.payKeySend(fixture.servicePublicKey, -2).isValid());
    CHECK(!lowLevel.multiPayInvoice({{"lnbc-valid", 1}, {"lnbc-invalid", 0}}).isValid());
    CHECK(!lowLevel.multiPayKeySend({{fixture.servicePublicKey, 1, "", {}},
                                     {fixture.servicePublicKey, -1, "", {}}}).isValid());
#if NOSTRDUINO_ENABLE_AMOUNT_GUARDS
    CHECK(!lowLevel.makeInvoice(nostr::MAX_MILLISATS + 1).isValid());
    CHECK(!lowLevel.makeInvoice(INT64_MAX).isValid());
#else
    CHECK(nostr::isRequiredMilliSatsValid(nostr::MAX_MILLISATS + 1));
    CHECK(nostr::isRequiredMilliSatsValid(INT64_MAX));
#endif

    nostr::esp32::ESP32Transport lnurlTransport;
    int lnurlCallbacks = 0;
    bool emptyLnurlInvoice = false;
    lnurlTransport.getInvoiceFromLNAddr("name@example.com", 0, "", [&](String invoice) {
        emptyLnurlInvoice = invoice.length() == 0;
        ++lnurlCallbacks;
    });
    lnurlTransport.getInvoiceFromLNAddr("name@example.com", -1, "", [&](String invoice) {
        emptyLnurlInvoice = emptyLnurlInvoice && invoice.length() == 0;
        ++lnurlCallbacks;
    });
    lnurlTransport.getInvoiceFromLNAddr("name@example.com", -2, "", [&](String invoice) {
        emptyLnurlInvoice = emptyLnurlInvoice && invoice.length() == 0;
        ++lnurlCallbacks;
    });
    CHECK(lnurlCallbacks == 3 && emptyLnurlInvoice);

    return true;
}

} // namespace unit_test
