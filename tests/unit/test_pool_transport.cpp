#include "test_unit.h"

#include <functional>
#include <vector>

#include "ArduinoJson.h"
#include "NostrEvent.h"
#include "NostrPool.h"
#include "NostrUtils.h"

namespace unit_test {

class MockConnection : public nostr::Connection {
  public:
    bool ready = true;
    std::vector<String> sent;
    std::vector<String> incoming;
    std::vector<std::function<void(String)>> messageListeners;

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
    void addConnectionStatusListener(std::function<void(nostr::ConnectionStatus)>) override {}
    void queue(String message) { incoming.push_back(message); }
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
    bool eoseCalled = false;
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
        [&](const String &id) { eoseCalled = id == subId; });
    CHECK(subId.length() == 64);
    CHECK(transport.lastConnection != nullptr);
    MockConnection *connection = transport.lastConnection;
    pool.loop();
    CHECK(!connection->sent.empty() && connection->sent.back().indexOf("\"REQ\"") >= 0);

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
    CHECK(eoseCalled);

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
    CHECK(pool.getRelays().size() == 1 && pool.getConnectedRelays()->size() == 1);
    pool.disconnectRelay("ws://relay.example");
    CHECK(pool.getRelays().empty() && transport.connections.empty());
    return true;
}

} // namespace unit_test
