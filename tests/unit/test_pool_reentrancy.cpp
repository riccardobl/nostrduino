#include "test_unit.h"

#include <functional>
#include <type_traits>
#include <vector>

#include "ArduinoJson.h"
#include "NostrEvent.h"
#include "NostrPool.h"
#include "NostrUtils.h"

namespace unit_test {
namespace {

class ReentrantConnection : public nostr::Connection {
  public:
    bool ready = true;
    std::vector<String> incoming;
    std::vector<std::function<void(String)>> messageListeners;
    std::vector<std::function<void(nostr::ConnectionStatus)>> statusListeners;

    void addMessageListener(std::function<void(String)> listener) override { messageListeners.push_back(listener); }
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
    void addConnectionStatusListener(std::function<void(nostr::ConnectionStatus)> listener) override {
        statusListeners.push_back(listener);
    }
};

class ReentrantTransport : public nostr::Transport {
  public:
    std::vector<ReentrantConnection *> connections;

    void getInvoiceFromLNAddr(String, nostr::MilliSats, String,
                              std::function<void(String)> callback) override {
        if (callback) callback("");
    }
    nostr::Connection *connect(String) override {
        ReentrantConnection *connection = new ReentrantConnection();
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
        for (ReentrantConnection *connection : connections) delete connection;
        connections.clear();
    }
    ~ReentrantTransport() override { close(); }
};

String eventFrame(const String &subId, const nostr::SignedNostrEvent &event) {
    JsonDocument document;
    JsonArray frame = document.to<JsonArray>();
    frame.add("EVENT");
    frame.add(subId);
    JsonObject object = frame.add<JsonObject>();
    event.toJson(object);
    String serialized;
    nostr::Utils::jsonStringify(frame, &serialized);
    return serialized;
}

} // namespace

bool testPoolReentrancy() {
    static_assert(!std::is_copy_constructible<nostr::NostrPool>::value, "NostrPool must not be copied");
    static_assert(!std::is_copy_assignable<nostr::NostrPool>::value, "NostrPool must not be copied");
    static_assert(!std::is_move_constructible<nostr::NostrPool>::value, "NostrPool must not be moved");
    static_assert(!std::is_move_assignable<nostr::NostrPool>::value, "NostrPool must not be moved");

    const String privateKey = repeatChar('0', 63) + "1";
    nostr::UnsignedNostrEvent unsignedEvent(1, "reentrant", 1700000000);
    nostr::SignedNostrEvent event = unsignedEvent.sign(privateKey);

    ReentrantTransport closeTransport;
    nostr::NostrPool closePool(&closeTransport);
    String closeSub = closePool.subscribeMany(
        {"ws://relay.example"}, {{{"kinds", {"1"}}}},
        [&](const String &, nostr::SignedNostrEvent *) { closePool.close(); });
    CHECK(closeTransport.connections.size() == 1);
    closeTransport.connections[0]->incoming.push_back(eventFrame(closeSub, event));
    closePool.loop();
    CHECK(closeTransport.connections.empty());
    CHECK(closePool.getConnectedRelays()->empty());
    CHECK(!closePool.hasSubscription(closeSub));

    ReentrantTransport disconnectTransport;
    nostr::NostrPool disconnectPool(&disconnectTransport);
    String disconnectSub = disconnectPool.subscribeMany(
        {"ws://relay.example"}, {{{"kinds", {"1"}}}},
        [&](const String &, nostr::SignedNostrEvent *) {
            disconnectPool.disconnectRelay("ws://relay.example");
        });
    disconnectTransport.connections[0]->incoming.push_back(eventFrame(disconnectSub, event));
    disconnectPool.loop();
    CHECK(disconnectTransport.connections.empty());
    CHECK(disconnectPool.getConnectedRelays()->empty());

    ReentrantTransport addTransport;
    nostr::NostrPool addPool(&addTransport);
    String addSub = addPool.subscribeMany(
        {"ws://relay-a.example"}, {{{"kinds", {"1"}}}},
        [&](const String &, nostr::SignedNostrEvent *) {
            addPool.ensureRelay("ws://relay-b.example");
        });
    addTransport.connections[0]->incoming.push_back(eventFrame(addSub, event));
    addPool.loop();
    CHECK(addTransport.connections.size() == 2);
    CHECK(addPool.getConnectedRelays()->size() == 2);

    ReentrantTransport eoseTransport;
    nostr::NostrPool eosePool(&eoseTransport);
    String eoseSub = eosePool.subscribeMany(
        {"ws://relay.example"}, {{{"kinds", {"1"}}}}, nullptr, nullptr,
        [&](const String &) { eosePool.close(); });
    eoseTransport.connections[0]->incoming.push_back(String("[\"EOSE\",\"") + eoseSub + "\"]");
    eosePool.loop();
    CHECK(eoseTransport.connections.empty());

    ReentrantTransport closedTransport;
    nostr::NostrPool closedPool(&closedTransport);
    String closedSub = closedPool.subscribeMany(
        {"ws://relay.example"}, {{{"kinds", {"1"}}}}, nullptr,
        [&](const String &, const String &) { closedPool.close(); });
    closedTransport.connections[0]->incoming.push_back(
        String("[\"CLOSED\",\"") + closedSub + "\",\"done\"]");
    closedPool.loop();
    CHECK(closedTransport.connections.empty());

    ReentrantTransport okTransport;
    nostr::NostrPool okPool(&okTransport);
    okPool.publish({"ws://relay.example"}, &event,
                   [&](const String &, bool, const String &) { okPool.close(); });
    okTransport.connections[0]->incoming.push_back(
        String("[\"OK\",\"") + event.getId() + "\",true,\"saved\"]");
    okPool.loop();
    CHECK(okTransport.connections.empty());
    return true;
}

} // namespace unit_test
