#include "test_unit.h"

#include "nwc_test_support.h"

namespace unit_test {

bool testNwcNotifications() {
    using namespace nwc_mock;

    const String notificationPreimage = repeatChar('a', 64);

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
                             notification.preimage == notificationPreimage &&
                             notification.paymentHash == "notification-payment-hash" &&
                             notification.amount == 10000 && notification.feesPaid == 100 &&
                             notification.createdAt == 800 && notification.settledAt == 900;
        },
        [&](String code, String message) {
            errorCode = code;
            errorMessage = message;
        });
    fixture.loop();
    const String notificationSub = fixture.subscriptionIdForKind(23197);
    CHECK(notificationSub.length() == 64);

    JsonDocument notification;
    notification["notification_type"] = "payment_received";
    JsonObject data = notification["notification"].to<JsonObject>();
    data["type"] = "incoming";
    data["invoice"] = "lnbc-notification";
    data["description"] = "notification description";
    data["description_hash"] = "notification-description-hash";
    data["preimage"] = notificationPreimage;
    data["payment_hash"] = "notification-payment-hash";
    data["amount"] = 10000;
    data["fees_paid"] = 100;
    data["created_at"] = 800;
    data["settled_at"] = 900;

    CHECK(fixture.queueNotification(notification, fixture.otherPrivateKey));
    CHECK(fixture.queueNotification(notification, "", 1));
    CHECK(fixture.queueNotification(notification, "", 23197, fixture.otherPublicKey));
    CHECK(fixture.queueNotification(notification));
    fixture.loop();
    CHECK(notificationCallbacks == 1);
    CHECK(notificationOk);
    CHECK(errorCode == "");

    data["amount"] = nostr::MAX_MILLISATS + 1;
    CHECK(fixture.queueNotification(notification));
    fixture.loop();
    CHECK(notificationCallbacks == 2);

    data["amount"] = "10000";
    CHECK(fixture.queueNotification(notification));
    fixture.loop();
    CHECK(notificationCallbacks == 2);
    CHECK(errorCode == "OTHER");
    data["amount"] = 1.5;
    CHECK(fixture.queueNotification(notification));
    fixture.loop();
    CHECK(notificationCallbacks == 2);
    data["amount"] = UINT64_MAX;
    CHECK(fixture.queueNotification(notification));
    fixture.loop();
    CHECK(notificationCallbacks == 2);
    data["amount"] = 10000;
    data["preimage"] = "bad-preimage";
    CHECK(fixture.queueNotification(notification));
    fixture.loop();
    CHECK(notificationCallbacks == 2);

    notification.clear();
    notification["error"]["code"] = "INTERNAL";
    notification["error"]["message"] = "wallet unavailable";
    CHECK(fixture.queueNotification(notification));
    fixture.loop();
    CHECK(notificationCallbacks == 2);
    CHECK(errorCode == "INTERNAL");
    CHECK(errorMessage == "wallet unavailable");

    const size_t sentBeforeReconnect = fixture.transport.connection->sent.size();
    fixture.transport.connection->status(nostr::ConnectionStatus::DISCONNECTED);
    fixture.transport.connection->status(nostr::ConnectionStatus::CONNECTED);
    fixture.loop();
    CHECK(fixture.transport.connection->sent.size() == sentBeforeReconnect + 1);
    CHECK(fixture.subscriptionIdForKind(23197) == notificationSub);

    setUnixTime(1700000000ULL + 1000000ULL);
    fixture.loop();
    CHECK(!fixture.subscriptionWasClosed(notificationSub));

    fixture.client.close();
    CHECK(fixture.transport.connection == nullptr);

    Fixture nip04Fixture(nostr::NostrLimits(), nostr::NWCEncryptionPolicy::AllowLegacyNip04);
    CHECK(nip04Fixture.negotiate("nip04"));
    int nip04Callbacks = 0;
    nip04Fixture.client.subscribeNotifications(
        [&](nostr::NotificationResponse) { ++nip04Callbacks; }, nullptr);
    nip04Fixture.loop();
    CHECK(nip04Fixture.subscriptionIdForKind(23196).length() == 64);
    CHECK(nip04Fixture.subscriptionIdForKind(23197) == "");
    notification.clear();
    notification["notification_type"] = "payment_received";
    notification["notification"]["type"] = "incoming";
    notification["notification"]["amount"] = 1;
    notification["notification"]["fees_paid"] = 0;
    notification["notification"]["created_at"] = 1;
    notification["notification"]["settled_at"] = 1;
    CHECK(nip04Fixture.queueNotification(notification, "", 23197));
    CHECK(nip04Fixture.queueNotification(notification));
    nip04Fixture.loop();
    CHECK(nip04Callbacks == 1);
    return true;
}

} // namespace unit_test
