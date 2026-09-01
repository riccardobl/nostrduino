#include "test_unit.h"

#include "nwc_test_support.h"

namespace unit_test {

bool testNwcResponses() {
    using namespace nwc_mock;

    const String payPreimage = repeatChar('a', 64);
    const String multiPreimageA = repeatChar('b', 64);
    const String multiPreimageB = repeatChar('c', 64);
    const String keysendPreimage = repeatChar('d', 64);
    const String multiKeysendPreimage = repeatChar('e', 64);
    const String invoicePreimage = repeatChar('f', 64);

    Fixture fixture;
    CHECK(fixture.negotiate());

    size_t requestIndex = fixture.requestCount();
    bool payInvoiceOk = false;
    fixture.client.payInvoice("lnbc-pay", 1, [&](nostr::PayInvoiceResponse result) {
        payInvoiceOk = result.preimage == payPreimage && result.feesPaid == 12;
    });
    fixture.loop();
    JsonDocument response;
    response["result_type"] = "pay_invoice";
    response["result"]["preimage"] = payPreimage;
    response["result"]["fees_paid"] = 12;
    CHECK(fixture.queueResponse(requestIndex, response));
    fixture.loop();
    CHECK(payInvoiceOk);

    requestIndex = fixture.requestCount();
    int multiInvoiceCallbacks = 0;
    String multiInvoiceDs;
    fixture.client.multiPayInvoice({{"lnbc-a", 1, "invoice-a"}, {"lnbc-b", 2, "invoice-b"}},
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
    response["result"]["preimage"] = multiPreimageA;
    CHECK(fixture.queueResponse(requestIndex, response, "invoice-a"));
    CHECK(fixture.queueResponse(requestIndex, response, "invoice-a"));
    response["result"]["preimage"] = multiPreimageB;
    CHECK(fixture.queueResponse(requestIndex, response, "invoice-b"));
    response["result"]["preimage"] = repeatChar('0', 64);
    CHECK(fixture.queueResponse(requestIndex, response, "invoice-duplicate"));
    fixture.loop();
    CHECK(multiInvoiceCallbacks == 2);
    CHECK(multiInvoiceDs.indexOf("invoice-a:" + multiPreimageA) >= 0);
    CHECK(multiInvoiceDs.indexOf("invoice-b:" + multiPreimageB) >= 0);
    fixture.loop();
    CHECK(fixture.subscriptionWasClosed(multiInvoiceSub));

    requestIndex = fixture.requestCount();
    bool keySendOk = false;
    fixture.client.payKeySend(fixture.servicePublicKey, 3, "", {},
        [&](nostr::PayKeySendResponse result) {
            keySendOk = result.preimage == keysendPreimage && result.feesPaid == 13;
        });
    fixture.loop();
    response.clear();
    response["result_type"] = "pay_keysend";
    response["result"]["preimage"] = keysendPreimage;
    response["result"]["fees_paid"] = 13;
    CHECK(fixture.queueResponse(requestIndex, response));
    fixture.loop();
    CHECK(keySendOk);

    requestIndex = fixture.requestCount();
    bool multiKeySendOk = false;
    fixture.client.multiPayKeySend({{fixture.servicePublicKey, 4, "", {}, "keysend-a"}},
        [&](nostr::MultiPayKeySendResponse result) {
            multiKeySendOk = result.preimage == multiKeysendPreimage && result.d == "keysend-a";
        });
    fixture.loop();
    response.clear();
    response["result_type"] = "multi_pay_keysend";
    response["result"]["preimage"] = multiKeysendPreimage;
    CHECK(fixture.queueResponse(requestIndex, response, "keysend-a"));
    fixture.loop();
    CHECK(multiKeySendOk);

    requestIndex = fixture.requestCount();
    bool makeInvoiceOk = false;
    fixture.client.makeInvoice(5, "memo", "", 60, [&](nostr::MakeInvoiceResponse result) {
        makeInvoiceOk = result.type == "incoming" && result.state == "pending" &&
                        result.invoice == "lnbc-created" &&
                        result.description == "created invoice" && result.descriptionHash == "desc-hash" &&
                        result.preimage == invoicePreimage && result.paymentHash == "created-hash" &&
                        result.amount == 5000 && result.feesPaid == 50 && result.createdAt == 100 &&
                        result.expiresAt == 200 && result.metadata["source"].as<String>() == "mock";
    });
    fixture.loop();
    response.clear();
    response["result_type"] = "make_invoice";
    JsonObject made = response["result"].to<JsonObject>();
    made["type"] = "incoming";
    made["state"] = "pending";
    made["invoice"] = "lnbc-created";
    made["description"] = "created invoice";
    made["description_hash"] = "desc-hash";
    made["preimage"] = invoicePreimage;
    made["payment_hash"] = "created-hash";
    made["amount"] = 5000;
    made["fees_paid"] = 50;
    made["created_at"] = 100;
    made["expires_at"] = 200;
    made["metadata"]["source"] = "mock";
    CHECK(fixture.queueResponse(requestIndex, response));
    fixture.loop();
    CHECK(makeInvoiceOk);

    requestIndex = fixture.requestCount();
    bool minimalInvoiceOk = false;
    fixture.client.makeInvoice(6, "", "", 0, [&](nostr::MakeInvoiceResponse result) {
        minimalInvoiceOk = result.type == "incoming" && result.invoice == "" &&
                           result.expiresAt == 0 && result.paymentHash == "minimal-hash";
    });
    fixture.loop();
    response.clear();
    response["result_type"] = "make_invoice";
    JsonObject minimalInvoice = response["result"].to<JsonObject>();
    minimalInvoice["type"] = "incoming";
    minimalInvoice["payment_hash"] = "minimal-hash";
    minimalInvoice["amount"] = 6000;
    minimalInvoice["fees_paid"] = 0;
    minimalInvoice["created_at"] = 101;
    CHECK(fixture.queueResponse(requestIndex, response));
    fixture.loop();
    CHECK(minimalInvoiceOk);

    auto fillLookupResponse = [&](JsonDocument &document, const char *invoice, const char *paymentHash) {
        document.clear();
        document["result_type"] = "lookup_invoice";
        JsonObject result = document["result"].to<JsonObject>();
        result["type"] = "outgoing";
        result["state"] = "settled";
        result["invoice"] = invoice;
        result["description"] = "lookup description";
        result["description_hash"] = "lookup-description-hash";
        result["preimage"] = invoicePreimage;
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
        lookupHashOk = result.type == "outgoing" && result.state == "settled" &&
                       result.invoice == "lnbc-by-hash" &&
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
                     result.transactions[0].state == "pending" &&
                     result.transactions[0].invoice == "lnbc-list" && result.transactions[0].amount == 8000 &&
                     result.transactions[0].settledAt == 600 &&
                     result.transactions[0].metadata["list"].as<String>() == "yes";
        });
    fixture.loop();
    response.clear();
    response["result_type"] = "list_transactions";
    JsonObject transaction = response["result"]["transactions"].to<JsonArray>().add<JsonObject>();
    transaction["type"] = "incoming";
    transaction["state"] = "pending";
    transaction["invoice"] = "lnbc-list";
    transaction["description"] = "listed";
    transaction["description_hash"] = "listed-description-hash";
    transaction["preimage"] = invoicePreimage;
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
                 result.notifications.size() == 1 && result.notifications[0] == "payment_received" &&
                 result.extensions.size() == 1 && result.extensions[0] == "02";
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
    info["extensions"].to<JsonArray>().add("02");
    CHECK(fixture.queueResponse(requestIndex, response));
    fixture.loop();
    CHECK(infoOk);

    nostr::Nip47 strictParser(nostr::Nip04(), fixture.servicePublicKey, fixture.userPrivateKey);
    strictParser.setEncryption(nostr::Nip47Encryption::Nip44V2, true);
    nostr::SignedNostrEvent parsedEvent;
    nostr::Nip47Response<nostr::GetInfoResponse> reusableInfo;
    CHECK(fixture.makeResponseEvent(requestIndex, response, parsedEvent));
    strictParser.parseResponse(&parsedEvent, reusableInfo);
    CHECK(reusableInfo.errorCode == "");
    CHECK(reusableInfo.resultType == "get_info");
    CHECK(reusableInfo.result.methods.size() == 2);
    response["result"].remove("notifications");
    CHECK(fixture.makeResponseEvent(requestIndex, response, parsedEvent));
    strictParser.parseResponse(&parsedEvent, reusableInfo);
    CHECK(reusableInfo.errorCode == "");
    CHECK(reusableInfo.result.notifications.empty());
    CHECK(reusableInfo.result.extensions.size() == 1);
    response["result"]["methods"].as<JsonArray>().add(7);
    CHECK(fixture.makeResponseEvent(requestIndex, response, parsedEvent));
    strictParser.parseResponse(&parsedEvent, reusableInfo);
    CHECK(reusableInfo.errorCode == "OTHER");
    CHECK(reusableInfo.result.methods.empty());
    CHECK(reusableInfo.result.notifications.empty());

    requestIndex = fixture.requestCount();
    fixture.client.getBalance();
    fixture.loop();
    nostr::Nip47Response<nostr::GetBalanceResponse> strictBalance;
    response.clear();
    response["result_type"] = "get_balance";
    response["result"]["balance"] = nostr::MAX_MILLISATS + 1;
    CHECK(fixture.makeResponseEvent(requestIndex, response, parsedEvent));
    strictParser.parseResponse(&parsedEvent, strictBalance);
    CHECK(strictBalance.errorCode == "");
    CHECK(strictBalance.result.balance == nostr::MAX_MILLISATS + 1);
    nostr::UnsignedNostrEvent attackerEnvelope(23195, parsedEvent.getContent(), 1700000000);
    attackerEnvelope.getTags()->addTag("p", {fixture.userPublicKey});
    nostr::SignedNostrEvent attackerResponse = attackerEnvelope.sign(fixture.otherPrivateKey);
    strictParser.parseResponse(&attackerResponse, strictBalance);
    CHECK(strictBalance.errorCode == "OTHER" && strictBalance.result.balance == 0);
    nostr::UnsignedNostrEvent wrongKindEnvelope(1, parsedEvent.getContent(), 1700000000);
    wrongKindEnvelope.getTags()->addTag("p", {fixture.userPublicKey});
    nostr::SignedNostrEvent wrongKindResponse = wrongKindEnvelope.sign(fixture.servicePrivateKey);
    strictParser.parseResponse(&wrongKindResponse, strictBalance);
    CHECK(strictBalance.errorCode == "OTHER" && strictBalance.result.balance == 0);

    response["result"]["balance"] = "9000";
    CHECK(fixture.makeResponseEvent(requestIndex, response, parsedEvent));
    strictParser.parseResponse(&parsedEvent, strictBalance);
    CHECK(strictBalance.errorCode == "OTHER" && strictBalance.result.balance == 0);
    response["result"]["balance"] = 1.5;
    CHECK(fixture.makeResponseEvent(requestIndex, response, parsedEvent));
    strictParser.parseResponse(&parsedEvent, strictBalance);
    CHECK(strictBalance.errorCode == "OTHER" && strictBalance.result.balance == 0);
    response["result"]["balance"] = -1;
    CHECK(fixture.makeResponseEvent(requestIndex, response, parsedEvent));
    strictParser.parseResponse(&parsedEvent, strictBalance);
    CHECK(strictBalance.errorCode == "OTHER" && strictBalance.result.balance == 0);
    response["result"]["balance"] = UINT64_MAX;
    CHECK(fixture.makeResponseEvent(requestIndex, response, parsedEvent));
    strictParser.parseResponse(&parsedEvent, strictBalance);
    CHECK(strictBalance.errorCode == "OTHER" && strictBalance.result.balance == 0);

    nostr::Nip47Response<nostr::PayInvoiceResponse> strictPayment;
    strictPayment.result.preimage = "stale";
    response.clear();
    response["result_type"] = "pay_invoice";
    response["result"]["preimage"] = "not-hex";
    CHECK(fixture.makeResponseEvent(0, response, parsedEvent));
    strictParser.parseResponse(&parsedEvent, strictPayment);
    CHECK(strictPayment.errorCode == "OTHER");
    CHECK(strictPayment.result.preimage == "");

    response.clear();
    response["result_type"] = "get_balance";
    response["error"]["code"] = 7;
    response["error"]["message"] = "bad";
    CHECK(fixture.makeResponseEvent(requestIndex, response, parsedEvent));
    strictParser.parseResponse(&parsedEvent, strictBalance);
    CHECK(strictBalance.errorCode == "OTHER");

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
