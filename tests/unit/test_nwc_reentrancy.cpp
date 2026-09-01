#include "test_unit.h"

#include "nwc_test_support.h"

namespace unit_test {

bool testNwcReentrancy() {
    using namespace nwc_mock;

    Fixture responseFixture;
    CHECK(responseFixture.negotiate());
    size_t requestIndex = responseFixture.requestCount();
    int firstResponseCalls = 0;
    int nestedResponseCalls = 0;
    responseFixture.client.payInvoice("lnbc-reentrant", 1, [&](nostr::PayInvoiceResponse) {
        ++firstResponseCalls;
        responseFixture.client.getBalance([&](nostr::GetBalanceResponse result) {
            if (result.balance == 22) ++nestedResponseCalls;
        });
    });
    responseFixture.loop();
    JsonDocument response;
    response["result_type"] = "pay_invoice";
    response["result"]["preimage"] = repeatChar('a', 64);
    CHECK(responseFixture.queueResponse(requestIndex, response));
    responseFixture.loop();
    CHECK(firstResponseCalls == 1);
    CHECK(responseFixture.requestCount() == 2);

    response.clear();
    response["result_type"] = "get_balance";
    response["result"]["balance"] = 22;
    CHECK(responseFixture.queueResponse(requestIndex + 1, response));
    responseFixture.loop();
    CHECK(nestedResponseCalls == 1);

    Fixture closeFixture;
    CHECK(closeFixture.negotiate());
    requestIndex = closeFixture.requestCount();
    int closeCalls = 0;
    closeFixture.client.payInvoice("lnbc-close", 1, [&](nostr::PayInvoiceResponse) {
        ++closeCalls;
        closeFixture.client.close();
    });
    closeFixture.loop();
    response.clear();
    response["result_type"] = "pay_invoice";
    response["result"]["preimage"] = repeatChar('b', 64);
    CHECK(closeFixture.queueResponse(requestIndex, response));
    closeFixture.loop();
    CHECK(closeCalls == 1);
    CHECK(closeFixture.transport.connection == nullptr);
    int closedErrors = 0;
    closeFixture.client.getBalance(nullptr, [&](String code, String) {
        if (code == "CLOSED") ++closedErrors;
    });
    closeFixture.client.payInvoice("lnbc-closed", 1, nullptr, [&](String code, String) {
        if (code == "CLOSED") ++closedErrors;
    });
    closeFixture.client.subscribeNotifications(nullptr, [&](String code, String) {
        if (code == "CLOSED") ++closedErrors;
    });
    CHECK(closedErrors == 3);

    Fixture encryptionFailureFixture;
    CHECK(encryptionFailureFixture.negotiate());
    const std::function<bool(uint8_t *, size_t)> randomProvider = nostr::Utils::randomBytesProvider;
    nostr::Utils::setRandomBytesProvider([](uint8_t *, size_t) { return false; });
    int encryptionErrors = 0;
    encryptionFailureFixture.client.getBalance(nullptr, [&](String code, String) {
        if (code == "ENCRYPTION_FAILED") ++encryptionErrors;
    });
    nostr::Utils::setRandomBytesProvider(randomProvider);
    CHECK(encryptionErrors == 1);

    Fixture timeoutFixture;
    CHECK(timeoutFixture.negotiate());
    int timeoutCalls = 0;
    timeoutFixture.client.getBalance(nullptr, [&](String code, String message) {
        if (code == "OTHER" && message == "timeout") ++timeoutCalls;
        timeoutFixture.client.getInfo();
    });
    timeoutFixture.loop();
    CHECK(timeoutFixture.requestCount() == 1);
    setUnixTime(1700000000ULL + NWC_DEFAULT_TIMEOUT + 1ULL);
    timeoutFixture.loop();
    CHECK(timeoutCalls == 1);
    timeoutFixture.loop();
    CHECK(timeoutFixture.requestCount() == 2);

    Fixture notificationFixture;
    CHECK(notificationFixture.negotiate());
    int originalListenerCalls = 0;
    int addedListenerCalls = 0;
    notificationFixture.client.subscribeNotifications(
        [&](nostr::NotificationResponse) {
            ++originalListenerCalls;
            notificationFixture.client.subscribeNotifications(
                [&](nostr::NotificationResponse) { ++addedListenerCalls; }, nullptr);
        },
        nullptr);
    notificationFixture.loop();

    JsonDocument notification;
    notification["notification_type"] = "payment_received";
    notification["notification"]["type"] = "incoming";
    notification["notification"]["amount"] = 1;
    notification["notification"]["fees_paid"] = 0;
    notification["notification"]["created_at"] = 1;
    notification["notification"]["settled_at"] = 1;
    CHECK(notificationFixture.queueNotification(notification));
    notificationFixture.loop();
    CHECK(originalListenerCalls == 1);
    CHECK(addedListenerCalls == 0);

    size_t notificationSubscriptions = 0;
    for (const String &message : notificationFixture.transport.connection->sent) {
        JsonDocument frameDocument;
        String copy = message;
        if (!nostr::Utils::jsonParse(&copy, &frameDocument) || !frameDocument.is<JsonArray>()) continue;
        JsonArray frame = frameDocument.as<JsonArray>();
        if (frame.size() >= 3 && frame[0].as<String>() == "REQ" &&
            frame[2]["kinds"][0].as<unsigned int>() == 23197) {
            ++notificationSubscriptions;
        }
    }
    CHECK(notificationSubscriptions == 2);

    return true;
}

} // namespace unit_test
