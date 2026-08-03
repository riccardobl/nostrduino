#include "test_unit.h"

#include "nwc_test_support.h"

namespace unit_test {

bool testNwcNotifications() {
    using namespace nwc_mock;

    Fixture fixture;
    CHECK(fixture.negotiate());

    int notificationCallbacks = 0;
    bool notificationOk = false;
    String errorCode;
    String errorMessage;
    fixture.client.subscribeNotifications(
        [&](nostr::NotificationResponse result) {
            ++notificationCallbacks;
            const nostr::Nip47Notification &notification = result.notification;
            notificationOk = notification.notificationType == "payment_received" &&
                             notification.type == "incoming" && notification.invoice == "lnbc-notification" &&
                             notification.description == "notification description" &&
                             notification.descriptionHash == "notification-description-hash" &&
                             notification.preimage == "notification-preimage" &&
                             notification.paymentHash == "notification-payment-hash" &&
                             notification.amount == 10000 && notification.feesPaid == 100 &&
                             notification.createdAt == 800 && notification.settledAt == 900;
        },
        [&](String code, String message) {
            errorCode = code;
            errorMessage = message;
        });
    fixture.loop();
    const String notificationSub = fixture.subscriptionIdForKind(23196);
    CHECK(notificationSub.length() == 64);

    JsonDocument notification;
    notification["notification_type"] = "payment_received";
    JsonObject data = notification["notification"].to<JsonObject>();
    data["type"] = "incoming";
    data["invoice"] = "lnbc-notification";
    data["description"] = "notification description";
    data["description_hash"] = "notification-description-hash";
    data["preimage"] = "notification-preimage";
    data["payment_hash"] = "notification-payment-hash";
    data["amount"] = 10000;
    data["fees_paid"] = 100;
    data["created_at"] = 800;
    data["settled_at"] = 900;

    CHECK(fixture.queueNotification(notification, fixture.otherPrivateKey));
    CHECK(fixture.queueNotification(notification, "", 1));
    CHECK(fixture.queueNotification(notification, "", 23196, fixture.otherPublicKey));
    CHECK(fixture.queueNotification(notification));
    fixture.loop();
    CHECK(notificationCallbacks == 1);
    CHECK(notificationOk);
    CHECK(errorCode == "");

    notification.clear();
    notification["error"]["code"] = "INTERNAL";
    notification["error"]["message"] = "wallet unavailable";
    CHECK(fixture.queueNotification(notification));
    fixture.loop();
    CHECK(notificationCallbacks == 1);
    CHECK(errorCode == "INTERNAL");
    CHECK(errorMessage == "wallet unavailable");

    setUnixTime(1700000000ULL + 1000000ULL);
    fixture.loop();
    CHECK(!fixture.subscriptionWasClosed(notificationSub));

    fixture.client.close();
    CHECK(fixture.transport.connection == nullptr);
    return true;
}

} // namespace unit_test
