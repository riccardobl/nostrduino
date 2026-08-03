#include "test_unit.h"

#include <functional>
#include <vector>

#include "ArduinoJson.h"
#include "NostrEvent.h"
#include "NostrPool.h"
#include "NostrUtils.h"

namespace unit_test {

class UtilsSubscriptionStateGuard {
  public:
    UtilsSubscriptionStateGuard()
        : randomBytesProvider(nostr::Utils::randomBytesProvider), subscriptions(nostr::Utils::subs) {}
    ~UtilsSubscriptionStateGuard() {
        nostr::Utils::setRandomBytesProvider(randomBytesProvider);
        nostr::Utils::subs = subscriptions;
    }

  private:
    std::function<bool(uint8_t *, size_t)> randomBytesProvider;
    unsigned long long subscriptions;
};

class MockConnection : public nostr::Connection {
  public:
    bool ready = false;
    std::vector<String> sent;
    std::vector<String> incoming;
    std::vector<std::function<void(String)>> messageListeners;
    std::vector<std::function<void(nostr::ConnectionStatus)>> statusListeners;

    void addMessageListener(std::function<void(String)> listener) override { messageListeners.push_back(listener); }
    void send(String message) override { sent.push_back(message); }
    void disconnect() override { ready = false; }
    void loop() override {
        std::vector<String> pending = incoming;
        incoming.clear();
        for (const String &message : pending) {
            for (const auto &listener : messageListeners) listener(message);
        }
    }
    bool isReady() override { return ready; }
    void addConnectionStatusListener(std::function<void(nostr::ConnectionStatus)> listener) override {
        statusListeners.push_back(listener);
    }
    void queue(String message) { incoming.push_back(message); }
    void status(nostr::ConnectionStatus value) {
        ready = value == nostr::ConnectionStatus::CONNECTED;
        for (const auto &listener : statusListeners) listener(value);
    }
};

class MockTransport : public nostr::Transport {
  public:
    std::vector<MockConnection *> connections;
    MockConnection *lastConnection = nullptr;

    void getInvoiceFromLNAddr(String, unsigned long long, String, std::function<void(String)> callback) override {
        if (callback) callback("");
    }
    nostr::Connection *connect(String) override {
        MockConnection *connection = new MockConnection();
        connections.push_back(connection);
        lastConnection = connection;
        return connection;
    }
    void disconnect(nostr::Connection *connection) override {
        for (size_t i = 0; i < connections.size(); ++i) {
            if (connections[i] == connection) {
                delete connections[i];
                connections.erase(connections.begin() + i);
                if (lastConnection == connection) lastConnection = nullptr;
                return;
            }
        }
    }
    bool isReady() override { return true; }
    void close() override {
        for (MockConnection *connection : connections) delete connection;
        connections.clear();
        lastConnection = nullptr;
    }
    ~MockTransport() override { close(); }
};
bool testPoolQueueAndTransport() {
    MockTransport transport;
    nostr::NostrPool pool(&transport, 600);
    int eoseCount = 0;
    bool eventCalled = false;
    bool storedEvent = false;
    bool statusCalled = false;
    bool statusOk = false;
    String subId = pool.subscribeMany(
        {"ws://relay.example"},
        {{{"kinds", {"1"}}, {"int since", {"2"}}, {"string #p", {"abc"}}}},
        [&](const String &, nostr::SignedNostrEvent *event) {
            eventCalled = event != nullptr;
            if (event != nullptr) storedEvent = event->isStored();
        },
        nullptr,
        [&](const String &id) {
            if (id == subId) ++eoseCount;
        });
    CHECK(subId.length() == 64);
    CHECK(pool.hasSubscription(subId));
    CHECK(pool.getSubscription(subId) != nullptr);
    CHECK(transport.lastConnection != nullptr);
    MockConnection *connection = transport.lastConnection;
    CHECK(connection->sent.empty());
    connection->status(nostr::ConnectionStatus::CONNECTED);
    pool.loop();
    CHECK(connection->sent.size() == 1 && connection->sent.back().indexOf("\"REQ\"") >= 0);

    nostr::UnsignedNostrEvent unsignedEvent(1, "pool event", 1700000000);
    nostr::SignedNostrEvent event = unsignedEvent.sign(
        "0000000000000000000000000000000000000000000000000000000000000001");
    JsonDocument eventDocument;
    JsonArray frame = eventDocument.to<JsonArray>();
    frame.add("EVENT");
    frame.add(subId);
    JsonObject eventObject = frame.add<JsonObject>();
    event.toJson(eventObject);
    String eventFrame;
    nostr::Utils::jsonStringify(frame, &eventFrame);
    connection->queue(eventFrame);
    pool.loop();
    CHECK(eventCalled && storedEvent);

    connection->queue(String("[\"EOSE\",\"") + subId + "\"]");
    pool.loop();
    CHECK(eoseCount == 1);
    CHECK(pool.getSubscription(subId)->eose);

    const size_t sentBeforeReconnect = connection->sent.size();
    connection->status(nostr::ConnectionStatus::DISCONNECTED);
    connection->status(nostr::ConnectionStatus::CONNECTED);
    CHECK(connection->sent.size() == sentBeforeReconnect);
    pool.loop();
    CHECK(connection->sent.size() == sentBeforeReconnect + 1);
    CHECK(!pool.getSubscription(subId)->eose);

    eventCalled = false;
    storedEvent = false;
    connection->queue(eventFrame);
    pool.loop();
    CHECK(eventCalled && storedEvent);
    connection->queue(String("[\"EOSE\",\"") + subId + "\"]");
    pool.loop();
    CHECK(eoseCount == 2);

    pool.publish({"ws://relay.example"}, &event,
                 [&](const String &id, bool ok, const String &message) {
                     statusCalled = id == event.getId() && message == "accepted";
                     statusOk = ok;
                 });
    pool.loop();
    CHECK(connection->sent.back().indexOf("\"EVENT\"") >= 0);
    connection->queue(String("[\"OK\",\"") + event.getId() + "\",true,\"accepted\"]");
    pool.loop();
    CHECK(statusCalled && statusOk);

    pool.closeSubscription(subId);
    pool.loop();
    CHECK(connection->sent.back().indexOf("\"CLOSE\"") >= 0);
    CHECK(!pool.hasSubscription(subId));
    CHECK(pool.getSubscription(subId) == nullptr);
    CHECK(pool.getRelays().size() == 1 && pool.getConnectedRelays()->size() == 1);
    pool.disconnectRelay("ws://relay.example");
    CHECK(pool.getRelays().empty() && transport.connections.empty());

    MockTransport multiRelayTransport;
    nostr::NostrPool multiRelayPool(&multiRelayTransport, 600);
    bool multiRelayStoredEvent = false;
    int multiRelayEventCount = 0;
    int multiRelayEoseCount = 0;
    String multiRelaySubId = multiRelayPool.subscribeMany(
        {"ws://relay-a.example", "ws://relay-b.example"},
        {{{"kinds", {"1"}}}},
        [&](const String &, nostr::SignedNostrEvent *receivedEvent) {
            ++multiRelayEventCount;
            multiRelayStoredEvent = receivedEvent != nullptr && receivedEvent->isStored();
        },
        nullptr,
        [&](const String &id) {
            if (id == multiRelaySubId) ++multiRelayEoseCount;
        });
    CHECK(multiRelayTransport.connections.size() == 2);
    MockConnection *relayA = multiRelayTransport.connections[0];
    MockConnection *relayB = multiRelayTransport.connections[1];
    relayA->status(nostr::ConnectionStatus::CONNECTED);
    relayB->status(nostr::ConnectionStatus::CONNECTED);
    multiRelayPool.loop();
    CHECK(relayA->sent.size() == 1 && relayB->sent.size() == 1);

    JsonDocument multiRelayEventDocument;
    JsonArray multiRelayEventArray = multiRelayEventDocument.to<JsonArray>();
    multiRelayEventArray.add("EVENT");
    multiRelayEventArray.add(multiRelaySubId);
    JsonObject multiRelayEventObject = multiRelayEventArray.add<JsonObject>();
    event.toJson(multiRelayEventObject);
    String multiRelayEventFrame;
    nostr::Utils::jsonStringify(multiRelayEventArray, &multiRelayEventFrame);

    relayA->queue(String("[\"EOSE\",\"") + multiRelaySubId + "\"]");
    multiRelayPool.loop();
    CHECK(multiRelayEoseCount == 0);
    CHECK(!multiRelayPool.getSubscription(multiRelaySubId)->eose);

    relayA->queue(multiRelayEventFrame);
    multiRelayPool.loop();
    CHECK(multiRelayEventCount == 1 && !multiRelayStoredEvent);
    relayB->queue(multiRelayEventFrame);
    multiRelayPool.loop();
    CHECK(multiRelayEventCount == 2 && multiRelayStoredEvent);

    relayB->queue(String("[\"EOSE\",\"") + multiRelaySubId + "\"]");
    multiRelayPool.loop();
    CHECK(multiRelayEoseCount == 1);
    CHECK(multiRelayPool.getSubscription(multiRelaySubId)->eose);

    relayA->status(nostr::ConnectionStatus::DISCONNECTED);
    relayA->status(nostr::ConnectionStatus::CONNECTED);
    multiRelayPool.loop();
    CHECK(relayA->sent.size() == 2);
    CHECK(relayB->sent.size() == 1);
    CHECK(!multiRelayPool.getSubscription(multiRelaySubId)->eose);

    relayB->queue(multiRelayEventFrame);
    multiRelayPool.loop();
    CHECK(multiRelayEventCount == 3 && !multiRelayStoredEvent);
    relayA->queue(multiRelayEventFrame);
    multiRelayPool.loop();
    CHECK(multiRelayEventCount == 4 && multiRelayStoredEvent);
    multiRelayPool.close();

    size_t randomSample = 0;
    UtilsSubscriptionStateGuard utilsStateGuard;
    nostr::Utils::subs = 1;
    nostr::Utils::setRandomBytesProvider([&](uint8_t *output, size_t size) {
        for (size_t i = 0; i < size; ++i) output[i] = randomSample++ == 0 ? 52 : 0;
        return true;
    });

    MockTransport collisionTransport;
    nostr::NostrPool collisionPool(&collisionTransport, 600);
    JsonDocument filtersDocument;
    JsonArray filters = filtersDocument.to<JsonArray>();
    String firstSubId = collisionPool.subscribeMany({}, filters);

    nostr::Utils::subs = 10;
    nostr::Utils::setRandomBytesProvider([](uint8_t *output, size_t size) {
        for (size_t i = 0; i < size; ++i) output[i] = 0;
        return true;
    });
    String regeneratedSubId = collisionPool.subscribeMany({}, filters);

    CHECK(firstSubId.startsWith("10"));
    CHECK(regeneratedSubId.startsWith("11"));
    CHECK(regeneratedSubId != firstSubId);
    return true;
}

} // namespace unit_test
