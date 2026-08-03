#ifndef NOSTRDUINO_NWC_TEST_SUPPORT_H
#define NOSTRDUINO_NWC_TEST_SUPPORT_H

#include <Arduino.h>
#include <functional>
#include <vector>

#include "ArduinoJson.h"
#include "NostrTransport.h"
#include "services/NWC.h"

namespace unit_test {
namespace nwc_mock {

class MockConnection : public nostr::Connection {
  public:
    bool ready = true;
    std::vector<String> sent;
    std::vector<String> incoming;
    std::vector<std::function<void(String)>> messageListeners;
    std::vector<std::function<void(nostr::ConnectionStatus)>> statusListeners;

    void addMessageListener(std::function<void(String)> listener) override;
    void send(String message) override;
    void disconnect() override;
    void loop() override;
    bool isReady() override;
    void addConnectionStatusListener(std::function<void(nostr::ConnectionStatus)> listener) override;
    void queue(const String &message);
    void status(nostr::ConnectionStatus value);
};

class MockTransport : public nostr::Transport {
  public:
    MockConnection *connection = nullptr;
    std::vector<String> connectedUrls;

    void getInvoiceFromLNAddr(String address, nostr::MilliSats amount, String comment,
                              std::function<void(String)> callback) override;
    nostr::Connection *connect(String url) override;
    void disconnect(nostr::Connection *connection) override;
    bool isReady() override;
    void close() override;
    ~MockTransport() override;
};

struct PublishedRequest {
    String eventId;
    String encryptedContent;
    String encryption;
};

void setUnixTime(unsigned long long value);

class Fixture {
  public:
    String userPrivateKey;
    String servicePrivateKey;
    String otherPrivateKey;
    String userPublicKey;
    String servicePublicKey;
    String otherPublicKey;
    MockTransport transport;
    nostr::NWC client;

    explicit Fixture(const nostr::NostrLimits &limits = nostr::NostrLimits());

    void loop();
    bool negotiate(const String &advertisedModes = "nip44_v2 nip04", bool includeEncryptionTag = true);
    bool queueInfoEvent(const String &advertisedModes, bool includeEncryptionTag = true,
                        const String &signingPrivateKey = "");
    String subscriptionIdForKind(unsigned int kind, const String &eventId = "") const;
    size_t requestCount() const;
    bool requestAt(size_t index, PublishedRequest &request) const;
    bool decryptRequest(size_t index, JsonDocument &document) const;
    bool queueResponse(size_t requestIndex, JsonDocument &response, const String &dTag = "",
                       const String &signingPrivateKey = "", unsigned int kind = 23195,
                       const String &targetPublicKey = "");
    bool makeResponseEvent(size_t requestIndex, JsonDocument &response,
                           nostr::SignedNostrEvent &event, const String &dTag = "");
    bool queueNotification(JsonDocument &notification, const String &signingPrivateKey = "",
                           unsigned int kind = 0, const String &targetPublicKey = "");
    bool subscriptionWasClosed(const String &subscriptionId) const;

  private:
    String encryptForClient(const String &plaintext, const String &encryption) const;
    bool queueWalletEvent(const String &subscriptionId, unsigned int kind, const String &content,
                          const String &eventReference, const String &dTag,
                          const String &signingPrivateKey, const String &targetPublicKey);
};

} // namespace nwc_mock
} // namespace unit_test

#endif
