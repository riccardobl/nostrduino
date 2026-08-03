#include "test_unit.h"

#include "nwc_test_support.h"

namespace unit_test {

bool testNwcRequests() {
    using namespace nwc_mock;

    Fixture fixture;
    fixture.client.payInvoice("lnbc-pay", 2100);
    fixture.client.multiPayInvoice({{"lnbc-a", 100}, {"lnbc-b", 200}});
    fixture.client.payKeySend(fixture.servicePublicKey, 3000, "preimage-a",
                              {{"5482373484", "value-a"}});
    fixture.client.multiPayKeySend({
        {fixture.servicePublicKey, 4000, "preimage-b", {{"1", "one"}}},
        {fixture.otherPublicKey, 5000, "", {{"2", "two"}}},
    });
    fixture.client.makeInvoice(6000, "description", "description-hash", 90);
    fixture.client.lookUpPaymentHash("payment-hash");
    fixture.client.lookUpInvoice("lnbc-lookup");
    fixture.client.listTransactions(10, 20, 30, 40, true, "incoming");
    fixture.client.getBalance();
    fixture.client.getInfo();
    fixture.client.subscribeNotifications(nullptr, nullptr);

    fixture.loop();
    CHECK(fixture.requestCount() == 0);
    CHECK(fixture.subscriptionIdForKind(23196) == "");
    CHECK(fixture.negotiate());
    CHECK(fixture.requestCount() == 10);
    CHECK(fixture.subscriptionIdForKind(23196).length() == 64);

    for (size_t index = 0; index < fixture.requestCount(); ++index) {
        PublishedRequest request;
        CHECK(fixture.requestAt(index, request));
        CHECK(request.encryption == "nip44_v2");
        CHECK(fixture.subscriptionIdForKind(23195, request.eventId).length() == 64);
    }

    JsonDocument request;
    CHECK(fixture.decryptRequest(0, request));
    CHECK(request["method"].as<String>() == "pay_invoice");
    CHECK(request["params"]["invoice"].as<String>() == "lnbc-pay");
    CHECK(request["params"]["amount"].as<unsigned long>() == 2100);

    CHECK(fixture.decryptRequest(1, request));
    CHECK(request["method"].as<String>() == "multi_pay_invoice");
    CHECK(request["params"]["invoices"].size() == 2);
    CHECK(request["params"]["invoices"][0]["invoice"].as<String>() == "lnbc-a");
    CHECK(request["params"]["invoices"][1]["amount"].as<unsigned long>() == 200);

    CHECK(fixture.decryptRequest(2, request));
    CHECK(request["method"].as<String>() == "pay_key_send");
    CHECK(request["params"]["pubkey"].as<String>() == fixture.servicePublicKey);
    CHECK(request["params"]["amount"].as<unsigned long>() == 3000);
    CHECK(request["params"]["preimage"].as<String>() == "preimage-a");
    CHECK(request["params"]["tlv_records"][0]["type"].as<String>() == "5482373484");
    CHECK(request["params"]["tlv_records"][0]["value"].as<String>() == "value-a");

    CHECK(fixture.decryptRequest(3, request));
    CHECK(request["method"].as<String>() == "multi_pay_key_send");
    CHECK(request["params"]["key_sends"].size() == 2);
    CHECK(request["params"]["key_sends"][0]["preimage"].as<String>() == "preimage-b");
    CHECK(request["params"]["key_sends"][0]["tlv_records"][0]["value"].as<String>() == "one");
    CHECK(request["params"]["key_sends"][1]["pubkey"].as<String>() == fixture.otherPublicKey);
    CHECK(request["params"]["key_sends"][1]["preimage"].isNull());
    CHECK(request["params"]["key_sends"][1]["tlv_records"][0]["type"].as<String>() == "2");

    CHECK(fixture.decryptRequest(4, request));
    CHECK(request["method"].as<String>() == "make_invoice");
    CHECK(request["params"]["amount"].as<unsigned long>() == 6000);
    CHECK(request["params"]["description"].as<String>() == "description");
    CHECK(request["params"]["description_hash"].as<String>() == "description-hash");
    CHECK(request["params"]["expiry"].as<unsigned long>() == 90);

    CHECK(fixture.decryptRequest(5, request));
    CHECK(request["method"].as<String>() == "lookup_invoice");
    CHECK(request["params"]["payment_hash"].as<String>() == "payment-hash");
    CHECK(request["params"]["invoice"].isNull());

    CHECK(fixture.decryptRequest(6, request));
    CHECK(request["method"].as<String>() == "lookup_invoice");
    CHECK(request["params"]["invoice"].as<String>() == "lnbc-lookup");
    CHECK(request["params"]["payment_hash"].isNull());

    CHECK(fixture.decryptRequest(7, request));
    CHECK(request["method"].as<String>() == "list_transactions");
    CHECK(request["params"]["from"].as<unsigned long>() == 10);
    CHECK(request["params"]["until"].as<unsigned long>() == 20);
    CHECK(request["params"]["limit"].as<int>() == 30);
    CHECK(request["params"]["offset"].as<int>() == 40);
    CHECK(request["params"]["unpaid"].as<bool>());
    CHECK(request["params"]["type"].as<String>() == "incoming");

    CHECK(fixture.decryptRequest(8, request));
    CHECK(request["method"].as<String>() == "get_balance");
    CHECK(request["params"].as<JsonObject>().size() == 0);

    CHECK(fixture.decryptRequest(9, request));
    CHECK(request["method"].as<String>() == "get_info");
    CHECK(request["params"].as<JsonObject>().size() == 0);

    return true;
}

} // namespace unit_test
