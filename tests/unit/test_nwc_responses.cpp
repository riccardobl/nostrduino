#include "test_unit.h"

#include "nwc_test_support.h"

namespace unit_test {

bool testNwcResponses() {
    using namespace nwc_mock;

    Fixture fixture;
    CHECK(fixture.negotiate());

    size_t requestIndex = fixture.requestCount();
    bool payInvoiceOk = false;
    fixture.client.payInvoice("lnbc-pay", 1, [&](nostr::PayInvoiceResponse result) {
        payInvoiceOk = result.preimage == "pay-preimage";
    });
    fixture.loop();
    JsonDocument response;
    response["result_type"] = "pay_invoice";
    response["result"]["preimage"] = "pay-preimage";
    CHECK(fixture.queueResponse(requestIndex, response));
    fixture.loop();
    CHECK(payInvoiceOk);

    requestIndex = fixture.requestCount();
    int multiInvoiceCallbacks = 0;
    String multiInvoiceDs;
    fixture.client.multiPayInvoice({{"lnbc-a", 1}, {"lnbc-b", 2}},
        [&](nostr::MultiPayInvoiceResponse result) {
            ++multiInvoiceCallbacks;
            multiInvoiceDs += result.d;
            multiInvoiceDs += ":";
            multiInvoiceDs += result.preimage;
            multiInvoiceDs += ";";
        });
    fixture.loop();
    PublishedRequest multiInvoiceRequest;
    CHECK(fixture.requestAt(requestIndex, multiInvoiceRequest));
    const String multiInvoiceSub = fixture.subscriptionIdForKind(23195, multiInvoiceRequest.eventId);
    response.clear();
    response["result_type"] = "multi_pay_invoice";
    response["result"]["preimage"] = "multi-preimage-a";
    CHECK(fixture.queueResponse(requestIndex, response, "invoice-a"));
    response["result"]["preimage"] = "multi-preimage-b";
    CHECK(fixture.queueResponse(requestIndex, response, "invoice-b"));
    response["result"]["preimage"] = "duplicate";
    CHECK(fixture.queueResponse(requestIndex, response, "invoice-duplicate"));
    fixture.loop();
    CHECK(multiInvoiceCallbacks == 2);
    CHECK(multiInvoiceDs.indexOf("invoice-a:multi-preimage-a") >= 0);
    CHECK(multiInvoiceDs.indexOf("invoice-b:multi-preimage-b") >= 0);
    fixture.loop();
    CHECK(fixture.subscriptionWasClosed(multiInvoiceSub));

    requestIndex = fixture.requestCount();
    bool keySendOk = false;
    fixture.client.payKeySend(fixture.servicePublicKey, 3, "", {},
        [&](nostr::PayKeySendResponse result) { keySendOk = result.preimage == "keysend-preimage"; });
    fixture.loop();
    response.clear();
    response["result_type"] = "pay_keysend";
    response["result"]["preimage"] = "keysend-preimage";
    CHECK(fixture.queueResponse(requestIndex, response));
    fixture.loop();
    CHECK(keySendOk);

    requestIndex = fixture.requestCount();
    bool multiKeySendOk = false;
    fixture.client.multiPayKeySend({{fixture.servicePublicKey, 4, "", {}}},
        [&](nostr::MultiPayKeySendResponse result) {
            multiKeySendOk = result.preimage == "multi-keysend-preimage" && result.d == "keysend-a";
        });
    fixture.loop();
    response.clear();
    response["result_type"] = "multi_pay_keysend";
    response["result"]["preimage"] = "multi-keysend-preimage";
    CHECK(fixture.queueResponse(requestIndex, response, "keysend-a"));
    fixture.loop();
    CHECK(multiKeySendOk);

    requestIndex = fixture.requestCount();
    bool makeInvoiceOk = false;
    fixture.client.makeInvoice(5, "memo", "", 60, [&](nostr::MakeInvoiceResponse result) {
        makeInvoiceOk = result.type == "incoming" && result.invoice == "lnbc-created" &&
                        result.description == "created invoice" && result.descriptionHash == "desc-hash" &&
                        result.preimage == "created-preimage" && result.paymentHash == "created-hash" &&
                        result.amount == 5000 && result.feesPaid == 50 && result.createdAt == 100 &&
                        result.expiresAt == 200 && result.metadata["source"].as<String>() == "mock";
    });
    fixture.loop();
    response.clear();
    response["result_type"] = "make_invoice";
    JsonObject made = response["result"].to<JsonObject>();
    made["type"] = "incoming";
    made["invoice"] = "lnbc-created";
    made["description"] = "created invoice";
    made["description_hash"] = "desc-hash";
    made["preimage"] = "created-preimage";
    made["payment_hash"] = "created-hash";
    made["amount"] = 5000;
    made["fees_paid"] = 50;
    made["created_at"] = 100;
    made["expires_at"] = 200;
    made["metadata"]["source"] = "mock";
    CHECK(fixture.queueResponse(requestIndex, response));
    fixture.loop();
    CHECK(makeInvoiceOk);

    auto fillLookupResponse = [&](JsonDocument &document, const char *invoice, const char *paymentHash) {
        document.clear();
        document["result_type"] = "lookup_invoice";
        JsonObject result = document["result"].to<JsonObject>();
        result["type"] = "outgoing";
        result["invoice"] = invoice;
        result["description"] = "lookup description";
        result["description_hash"] = "lookup-description-hash";
        result["preimage"] = "lookup-preimage";
        result["payment_hash"] = paymentHash;
        result["amount"] = 7000;
        result["fees_paid"] = 70;
        result["created_at"] = 300;
        result["expires_at"] = 400;
        result["settled_at"] = 350;
        result["metadata"]["lookup"] = "yes";
    };

    requestIndex = fixture.requestCount();
    bool lookupHashOk = false;
    fixture.client.lookUpPaymentHash("lookup-hash", [&](nostr::LookUpInvoiceResponse result) {
        lookupHashOk = result.type == "outgoing" && result.invoice == "lnbc-by-hash" &&
                       result.paymentHash == "lookup-hash" && result.amount == 7000 &&
                       result.settledAt == 350 && result.metadata["lookup"].as<String>() == "yes";
    });
    fixture.loop();
    fillLookupResponse(response, "lnbc-by-hash", "lookup-hash");
    CHECK(fixture.queueResponse(requestIndex, response));
    fixture.loop();
    CHECK(lookupHashOk);

    requestIndex = fixture.requestCount();
    bool lookupInvoiceOk = false;
    fixture.client.lookUpInvoice("lnbc-by-invoice", [&](nostr::LookUpInvoiceResponse result) {
        lookupInvoiceOk = result.invoice == "lnbc-by-invoice" && result.paymentHash == "invoice-hash" &&
                          result.descriptionHash == "lookup-description-hash" && result.feesPaid == 70 &&
                          result.expiresAt == 400;
    });
    fixture.loop();
    fillLookupResponse(response, "lnbc-by-invoice", "invoice-hash");
    CHECK(fixture.queueResponse(requestIndex, response));
    fixture.loop();
    CHECK(lookupInvoiceOk);

    requestIndex = fixture.requestCount();
    bool listOk = false;
    fixture.client.listTransactions(0, 0, 0, 0, false, "",
        [&](nostr::ListTransactionsResponse result) {
            listOk = result.transactions.size() == 1 && result.transactions[0].type == "incoming" &&
                     result.transactions[0].invoice == "lnbc-list" && result.transactions[0].amount == 8000 &&
                     result.transactions[0].settledAt == 600 &&
                     result.transactions[0].metadata["list"].as<String>() == "yes";
        });
    fixture.loop();
    response.clear();
    response["result_type"] = "list_transactions";
    JsonObject transaction = response["result"]["transactions"].to<JsonArray>().add<JsonObject>();
    transaction["type"] = "incoming";
    transaction["invoice"] = "lnbc-list";
    transaction["description"] = "listed";
    transaction["description_hash"] = "listed-description-hash";
    transaction["preimage"] = "listed-preimage";
    transaction["payment_hash"] = "listed-hash";
    transaction["amount"] = 8000;
    transaction["fees_paid"] = 80;
    transaction["created_at"] = 500;
    transaction["expires_at"] = 700;
    transaction["settled_at"] = 600;
    transaction["metadata"]["list"] = "yes";
    CHECK(fixture.queueResponse(requestIndex, response));
    fixture.loop();
    CHECK(listOk);

    requestIndex = fixture.requestCount();
    int balanceCallbacks = 0;
    nostr::MilliSats balance = 0;
    fixture.client.getBalance([&](nostr::GetBalanceResponse result) {
        ++balanceCallbacks;
        balance = result.balance;
    });
    fixture.loop();
    PublishedRequest balanceRequest;
    CHECK(fixture.requestAt(requestIndex, balanceRequest));
    const String balanceSub = fixture.subscriptionIdForKind(23195, balanceRequest.eventId);
    response.clear();
    response["result_type"] = "get_balance";
    response["result"]["balance"] = 9000;
    CHECK(fixture.queueResponse(requestIndex, response, "", fixture.otherPrivateKey));
    CHECK(fixture.queueResponse(requestIndex, response, "", "", 1));
    CHECK(fixture.queueResponse(requestIndex, response, "", "", 23195, fixture.otherPublicKey));
    CHECK(fixture.queueResponse(requestIndex, response));
    CHECK(fixture.queueResponse(requestIndex, response));
    fixture.loop();
    CHECK(balanceCallbacks == 1 && balance == 9000);
    fixture.loop();
    CHECK(fixture.subscriptionWasClosed(balanceSub));

    requestIndex = fixture.requestCount();
    bool infoOk = false;
    fixture.client.getInfo([&](nostr::GetInfoResponse result) {
        infoOk = result.alias == "mock-wallet" && result.color == "#123456" &&
                 result.pubkey == fixture.servicePublicKey && result.network == "regtest" &&
                 result.blockHeight == 42 && result.blockHash == "block-hash" &&
                 result.methods.size() == 2 && result.methods[0] == "pay_invoice" &&
                 result.notifications.size() == 1 && result.notifications[0] == "payment_received";
    });
    fixture.loop();
    response.clear();
    response["result_type"] = "get_info";
    JsonObject info = response["result"].to<JsonObject>();
    info["alias"] = "mock-wallet";
    info["color"] = "#123456";
    info["pubkey"] = fixture.servicePublicKey;
    info["network"] = "regtest";
    info["block_height"] = 42;
    info["block_hash"] = "block-hash";
    info["methods"].to<JsonArray>().add("pay_invoice");
    info["methods"].as<JsonArray>().add("get_balance");
    info["notifications"].to<JsonArray>().add("payment_received");
    CHECK(fixture.queueResponse(requestIndex, response));
    fixture.loop();
    CHECK(infoOk);

    requestIndex = fixture.requestCount();
    String walletErrorCode;
    String walletErrorMessage;
    fixture.client.getBalance(nullptr, [&](String code, String message) {
        walletErrorCode = code;
        walletErrorMessage = message;
    });
    fixture.loop();
    response.clear();
    response["result_type"] = "get_balance";
    response["error"]["code"] = "RATE_LIMITED";
    response["error"]["message"] = "slow down";
    CHECK(fixture.queueResponse(requestIndex, response));
    fixture.loop();
    CHECK(walletErrorCode == "RATE_LIMITED" && walletErrorMessage == "slow down");

    requestIndex = fixture.requestCount();
    String timeoutCode;
    String timeoutMessage;
    fixture.client.getBalance(nullptr, [&](String code, String message) {
        timeoutCode = code;
        timeoutMessage = message;
    });
    fixture.loop();
    PublishedRequest timeoutRequest;
    CHECK(fixture.requestAt(requestIndex, timeoutRequest));
    const String timeoutSub = fixture.subscriptionIdForKind(23195, timeoutRequest.eventId);
    setUnixTime(1700000000ULL + NWC_DEFAULT_TIMEOUT + 1ULL);
    fixture.loop();
    CHECK(timeoutCode == "OTHER" && timeoutMessage == "timeout");
    fixture.loop();
    CHECK(fixture.subscriptionWasClosed(timeoutSub));

    return true;
}

} // namespace unit_test
