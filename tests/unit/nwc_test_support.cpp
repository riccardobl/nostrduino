#include "nwc_test_support.h"

#include "Nip04.h"
#include "Nip44.h"
#include "NostrEvent.h"
#include "NostrUtils.h"

namespace unit_test {
namespace nwc_mock {

namespace {

unsigned long long currentUnixTime = 1700000000ULL;

bool parseFrame(const String &message, JsonDocument &document, JsonArray &frame) {
    String copy = message;
    if (!nostr::Utils::jsonParse(&copy, &document) || !document.is<JsonArray>()) return false;
    frame = document.as<JsonArray>();
    return true;
}

String makeNwcUrl(const String &servicePublicKey, const String &userPrivateKey) {
    return "nostr+walletconnect://" + servicePublicKey +
           "?relay=wss%3A%2F%2Frelay.example&secret=" + userPrivateKey;
}

} // namespace

void MockConnection::addMessageListener(std::function<void(String)> listener) {
    messageListeners.push_back(listener);
}

void MockConnection::send(String message) {
    sent.push_back(message);
}

void MockConnection::disconnect() {
    ready = false;
}

void MockConnection::loop() {
    std::vector<String> pending;
    pending.swap(incoming);
    for (const String &message : pending) {
        for (const auto &listener : messageListeners) listener(message);
    }
}

bool MockConnection::isReady() {
    return ready;
}

void MockConnection::addConnectionStatusListener(std::function<void(nostr::ConnectionStatus)>) {}

void MockConnection::queue(const String &message) {
    incoming.push_back(message);
}

void MockTransport::getInvoiceFromLNAddr(String, nostr::MilliSats, String,
                                         std::function<void(String)> callback) {
    if (callback) callback("");
}

nostr::Connection *MockTransport::connect(String) {
    if (connection == nullptr) connection = new MockConnection();
    return connection;
}

void MockTransport::disconnect(nostr::Connection *target) {
    if (target != connection) return;
    delete connection;
    connection = nullptr;
}

bool MockTransport::isReady() {
    return true;
}

void MockTransport::close() {
    delete connection;
    connection = nullptr;
}

MockTransport::~MockTransport() {
    close();
}

void setUnixTime(unsigned long long value) {
    currentUnixTime = value;
    nostr::Utils::setUnixTimeSecondsProvider([]() -> unsigned long long { return currentUnixTime; });
}

Fixture::Fixture()
    : userPrivateKey("0000000000000000000000000000000000000000000000000000000000000001"),
      servicePrivateKey("0000000000000000000000000000000000000000000000000000000000000002"),
      otherPrivateKey("0000000000000000000000000000000000000000000000000000000000000003"),
      userPublicKey(nostr::Utils::getPublicKey(userPrivateKey)),
      servicePublicKey(nostr::Utils::getPublicKey(servicePrivateKey)),
      otherPublicKey(nostr::Utils::getPublicKey(otherPrivateKey)),
      transport(), client(&transport, makeNwcUrl(servicePublicKey, userPrivateKey)) {
    setUnixTime(1700000000ULL);
}

void Fixture::loop() {
    client.loop();
}

String Fixture::subscriptionIdForKind(unsigned int kind, const String &eventId) const {
    if (transport.connection == nullptr) return "";
    for (const String &message : transport.connection->sent) {
        JsonDocument document;
        JsonArray frame;
        if (!parseFrame(message, document, frame) || frame.size() < 3 ||
            frame[0].as<String>() != "REQ") continue;
        for (size_t filterIndex = 2; filterIndex < frame.size(); ++filterIndex) {
            JsonObject filter = frame[filterIndex].as<JsonObject>();
            if (filter["kinds"][0].as<unsigned int>() != kind) continue;
            if (eventId.length() > 0 && filter["#e"][0].as<String>() != eventId) continue;
            return frame[1].as<String>();
        }
    }
    return "";
}

bool Fixture::queueInfoEvent(const String &advertisedModes, bool includeEncryptionTag,
                             const String &signingPrivateKey) {
    loop();
    const String subscriptionId = subscriptionIdForKind(13194);
    if (subscriptionId.length() != 64 || transport.connection == nullptr) return false;
    nostr::UnsignedNostrEvent unsignedInfo(
        13194,
        "pay_invoice multi_pay_invoice pay_keysend multi_pay_keysend make_invoice lookup_invoice list_transactions get_balance get_info",
        1700000000);
    if (includeEncryptionTag) unsignedInfo.getTags()->addTag("encryption", {advertisedModes});
    String signer = signingPrivateKey.length() > 0 ? signingPrivateKey : servicePrivateKey;
    nostr::SignedNostrEvent info = unsignedInfo.sign(signer);
    JsonDocument document;
    JsonArray frame = document.to<JsonArray>();
    frame.add("EVENT");
    frame.add(subscriptionId);
    JsonObject eventObject = frame.add<JsonObject>();
    info.toJson(eventObject);
    String serialized;
    nostr::Utils::jsonStringify(frame, &serialized);
    transport.connection->queue(serialized);
    return true;
}

bool Fixture::negotiate(const String &advertisedModes, bool includeEncryptionTag) {
    if (!queueInfoEvent(advertisedModes, includeEncryptionTag)) return false;
    loop();
    return client.isReady();
}

size_t Fixture::requestCount() const {
    if (transport.connection == nullptr) return 0;
    size_t count = 0;
    for (const String &message : transport.connection->sent) {
        JsonDocument document;
        JsonArray frame;
        if (!parseFrame(message, document, frame) || frame.size() != 2 ||
            frame[0].as<String>() != "EVENT") continue;
        if (frame[1]["kind"].as<unsigned int>() == 23194) ++count;
    }
    return count;
}

bool Fixture::requestAt(size_t index, PublishedRequest &request) const {
    if (transport.connection == nullptr) return false;
    size_t current = 0;
    for (const String &message : transport.connection->sent) {
        JsonDocument document;
        JsonArray frame;
        if (!parseFrame(message, document, frame) || frame.size() != 2 ||
            frame[0].as<String>() != "EVENT") continue;
        JsonObject event = frame[1].as<JsonObject>();
        if (event["kind"].as<unsigned int>() != 23194) continue;
        if (current++ != index) continue;
        request.eventId = event["id"].as<String>();
        request.encryptedContent = event["content"].as<String>();
        request.encryption = "";
        for (JsonArray tag : event["tags"].as<JsonArray>()) {
            if (tag.size() == 2 && tag[0].as<String>() == "encryption") {
                request.encryption = tag[1].as<String>();
            }
        }
        return request.eventId.length() == 64 && request.encryptedContent.length() > 0;
    }
    return false;
}

bool Fixture::decryptRequest(size_t index, JsonDocument &document) const {
    PublishedRequest request;
    if (!requestAt(index, request)) return false;
    String privateKey = servicePrivateKey;
    String publicKey = userPublicKey;
    String content = request.encryptedContent;
    String plaintext;
    if (request.encryption == "nip44_v2") {
        nostr::Nip44 nip44;
        plaintext = nip44.decrypt(privateKey, publicKey, content);
    } else {
        nostr::Nip04 nip04;
        plaintext = nip04.decrypt(privateKey, publicKey, content);
    }
    document.clear();
    return plaintext.length() > 0 && nostr::Utils::jsonParse(&plaintext, &document);
}

String Fixture::encryptForClient(const String &plaintext, const String &encryption) const {
    String privateKey = servicePrivateKey;
    String publicKey = userPublicKey;
    if (encryption == "nip44_v2") {
        nostr::Nip44 nip44;
        return nip44.encrypt(privateKey, publicKey, plaintext);
    }
    nostr::Nip04 nip04;
    return nip04.encrypt(privateKey, publicKey, plaintext);
}

bool Fixture::queueWalletEvent(const String &subscriptionId, unsigned int kind, const String &content,
                               const String &eventReference, const String &dTag,
                               const String &signingPrivateKey, const String &targetPublicKey) {
    if (subscriptionId.length() != 64 || transport.connection == nullptr) return false;
    nostr::UnsignedNostrEvent unsignedEvent(kind, content, currentUnixTime);
    unsignedEvent.getTags()->addTag("p", {targetPublicKey});
    if (eventReference.length() > 0) unsignedEvent.getTags()->addTag("e", {eventReference});
    if (dTag.length() > 0) unsignedEvent.getTags()->addTag("d", {dTag});
    nostr::SignedNostrEvent event = unsignedEvent.sign(signingPrivateKey);
    JsonDocument document;
    JsonArray frame = document.to<JsonArray>();
    frame.add("EVENT");
    frame.add(subscriptionId);
    JsonObject eventObject = frame.add<JsonObject>();
    event.toJson(eventObject);
    String serialized;
    nostr::Utils::jsonStringify(frame, &serialized);
    transport.connection->queue(serialized);
    return true;
}

bool Fixture::queueResponse(size_t requestIndex, JsonDocument &response, const String &dTag,
                            const String &signingPrivateKey, unsigned int kind,
                            const String &targetPublicKey) {
    PublishedRequest request;
    if (!requestAt(requestIndex, request)) return false;
    const String subscriptionId = subscriptionIdForKind(23195, request.eventId);
    String plaintext;
    nostr::Utils::jsonStringify(response.as<JsonVariantConst>(), &plaintext);
    String signer = signingPrivateKey.length() > 0 ? signingPrivateKey : servicePrivateKey;
    String target = targetPublicKey.length() > 0 ? targetPublicKey : userPublicKey;
    return queueWalletEvent(subscriptionId, kind, encryptForClient(plaintext, request.encryption),
                            request.eventId, dTag, signer, target);
}

bool Fixture::queueNotification(JsonDocument &notification, const String &signingPrivateKey,
                                unsigned int kind, const String &targetPublicKey) {
    const String subscriptionId = subscriptionIdForKind(23196);
    String plaintext;
    nostr::Utils::jsonStringify(notification.as<JsonVariantConst>(), &plaintext);
    String signer = signingPrivateKey.length() > 0 ? signingPrivateKey : servicePrivateKey;
    String target = targetPublicKey.length() > 0 ? targetPublicKey : userPublicKey;
    const String encryption = client.getEncryption() == nostr::Nip47Encryption::Nip44V2
                                  ? "nip44_v2"
                                  : "nip04";
    return queueWalletEvent(subscriptionId, kind, encryptForClient(plaintext, encryption),
                            "", "", signer, target);
}

bool Fixture::subscriptionWasClosed(const String &subscriptionId) const {
    if (transport.connection == nullptr) return false;
    for (const String &message : transport.connection->sent) {
        JsonDocument document;
        JsonArray frame;
        if (!parseFrame(message, document, frame) || frame.size() != 2) continue;
        if (frame[0].as<String>() == "CLOSE" && frame[1].as<String>() == subscriptionId) return true;
    }
    return false;
}

} // namespace nwc_mock
} // namespace unit_test
