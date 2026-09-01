#include "test_unit.h"

#include <functional>
#include <vector>

#include "ArduinoJson.h"
#include "NostrEvent.h"
#include "NostrPool.h"
#include "NostrUtils.h"
#include "nwc_test_support.h"

namespace unit_test {
namespace {

class TrackerConnection : public nostr::Connection {
  public:
    bool ready = true;
    std::vector<String> incoming;
    std::vector<std::function<void(String)>> messageListeners;
    std::vector<std::function<void(nostr::ConnectionStatus)>> statusListeners;

    void addMessageListener(std::function<void(String)> listener) override {
        messageListeners.push_back(listener);
    }
    void send(String) override {}
    void disconnect() override { ready = false; }
    void loop() override {
        std::vector<String> pending;
        pending.swap(incoming);
        for (const String &message : pending) {
            for (const auto &listener : messageListeners) listener(message);
        }
    }
    bool isReady() override { return ready; }
    void addConnectionStatusListener(
        std::function<void(nostr::ConnectionStatus)> listener) override {
        statusListeners.push_back(listener);
    }
};

class TrackerTransport : public nostr::Transport {
  public:
    std::vector<TrackerConnection *> connections;

    void getInvoiceFromLNAddr(String, nostr::MilliSats, String,
                              std::function<void(String)> callback) override {
        if (callback) callback("");
    }
    nostr::Connection *connect(String) override {
        TrackerConnection *connection = new TrackerConnection();
        connections.push_back(connection);
        return connection;
    }
    void disconnect(nostr::Connection *target) override {
        for (size_t i = 0; i < connections.size(); ++i) {
            if (connections[i] != target) continue;
            delete connections[i];
            connections.erase(connections.begin() + i);
            return;
        }
    }
    bool isReady() override { return true; }
    void close() override {
        for (TrackerConnection *connection : connections) delete connection;
        connections.clear();
    }
    ~TrackerTransport() override { close(); }
};

String trackerEventFrame(const String &subId, const nostr::SignedNostrEvent &event,
                         bool validSignature = true) {
    JsonDocument document;
    JsonArray frame = document.to<JsonArray>();
    frame.add("EVENT");
    frame.add(subId);
    JsonObject object = frame.add<JsonObject>();
    event.toJson(object);
    if (!validSignature) object["sig"] = repeatChar('0', 128);
    String serialized;
    nostr::Utils::jsonStringify(frame, &serialized);
    return serialized;
}

bool trackEventId(const nostr::SignedNostrEvent &event, std::vector<String> &seenIds) {
    for (const String &id : seenIds) {
        if (id == event.getId()) return true;
    }
    seenIds.push_back(event.getId());
    return false;
}

} // namespace

bool testSeenTracker() {
    const String privateKey = repeatChar('0', 63) + "1";
    nostr::SignedNostrEvent event =
        nostr::UnsignedNostrEvent(1, "tracked event", 1700000000).sign(privateKey);
    CHECK(event.isValid());

    TrackerTransport transport;
    nostr::NostrPool pool(&transport);
    int deliveries = 0;
    String subId = pool.subscribeMany(
        {"ws://relay-one.example", "ws://relay-two.example"},
        {{{"kinds", {"1"}}}},
        [&](const String &, nostr::SignedNostrEvent *) { ++deliveries; });
    CHECK(subId.length() == 64);
    CHECK(transport.connections.size() == 2);

    const String frame = trackerEventFrame(subId, event);
    transport.connections[0]->incoming.push_back(frame);
    transport.connections[1]->incoming.push_back(frame);
    pool.loop();
    CHECK(deliveries == 2);

    std::vector<String> seenIds;
    int trackerCalls = 0;
    pool.setSeenTracker([&](const nostr::SignedNostrEvent &received) {
        ++trackerCalls;
        return trackEventId(received, seenIds);
    });
    transport.connections[0]->incoming.push_back(frame);
    transport.connections[1]->incoming.push_back(frame);
    pool.loop();
    CHECK(trackerCalls == 2);
    CHECK(deliveries == 3);

    pool.setVerifyIncomingEvents(true);
    transport.connections[0]->incoming.push_back(trackerEventFrame(subId, event, false));
    pool.loop();
    CHECK(trackerCalls == 2);
    CHECK(deliveries == 3);

    int replacementCalls = 0;
    pool.setSeenTracker([&](const nostr::SignedNostrEvent &) {
        ++replacementCalls;
        return true;
    });
    transport.connections[0]->incoming.push_back(frame);
    pool.loop();
    CHECK(replacementCalls == 1);
    CHECK(deliveries == 3);

    pool.setSeenTracker(nullptr);
    transport.connections[0]->incoming.push_back(frame);
    transport.connections[1]->incoming.push_back(frame);
    pool.loop();
    CHECK(deliveries == 5);

    nwc_mock::Fixture fixture;
    CHECK(fixture.negotiate());
    std::vector<String> notificationIds;
    int nwcTrackerCalls = 0;
    int notificationCalls = 0;
    fixture.client.setSeenTracker([&](const nostr::SignedNostrEvent &received) {
        ++nwcTrackerCalls;
        return trackEventId(received, notificationIds);
    });
    fixture.client.subscribeNotifications(
        [&](nostr::NotificationResponse) { ++notificationCalls; }, nullptr);
    fixture.loop();

    JsonDocument notification;
    notification["notification_type"] = "payment_received";
    notification["notification"]["type"] = "incoming";
    notification["notification"]["amount"] = 1;
    notification["notification"]["fees_paid"] = 0;
    notification["notification"]["created_at"] = 1;
    notification["notification"]["settled_at"] = 1;
    CHECK(fixture.queueNotification(notification));
    CHECK(!fixture.transport.connection->incoming.empty());
    const String trackedReplay = fixture.transport.connection->incoming.back();
    fixture.transport.connection->queue(trackedReplay);
    fixture.loop();
    CHECK(nwcTrackerCalls == 2);
    CHECK(notificationCalls == 1);

    fixture.client.setSeenTracker(nullptr);
    CHECK(fixture.queueNotification(notification));
    CHECK(!fixture.transport.connection->incoming.empty());
    const String untrackedReplay = fixture.transport.connection->incoming.back();
    fixture.transport.connection->queue(untrackedReplay);
    fixture.loop();
    CHECK(notificationCalls == 3);

    return true;
}

} // namespace unit_test
