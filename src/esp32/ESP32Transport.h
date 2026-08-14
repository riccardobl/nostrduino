#ifndef _NOSTR_ESP32_TRANSPORT_H
#define _NOSTR_ESP32_TRANSPORT_H 1
#include "NostrCommon.h"

#if defined(_ESP32_BOARD_)

#include "ArduinoJson.h"
#include "NostrString.h"
#include "NostrLimits.h"
#include "NostrTransport.h"
#include "NostrUrl.h"
#include "NostrUtils.h"
#include "WiFi.h"
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <WebSocketsClient.h>
#include <functional>
#include <initializer_list>
#include <vector>
namespace nostr {
namespace esp32 {
struct ESP32TlsConfig {
    // The caller owns this PEM buffer and must keep it alive for the complete
    // lifetime of the transport and every connection created from it.
    const char *caCertificatePem;
    bool requireCertificateValidation;

    ESP32TlsConfig() : caCertificatePem(nullptr), requireCertificateValidation(false) {}
};

struct ESP32TransportConfig {
    /**
     * Lightning Address responses are checked for the required LNURL-pay
     * fields, and the returned BOLT11 invoice is validated for checksum,
     * signature, amount and expiry. Network binding and callback-host
     * filtering are optional.
     *
     * Example:
     * @code
     * nostr::esp32::ESP32TransportConfig config;
     * config.expectedBolt11Currency = "bc"; // mainnet; empty accepts any standard network
     * config.lnurlCallbackFilter = [](const nostr::ParsedUrl &url) {
     *     return url.host == "pay.example.com" && url.port == 443;
     * };
     * @endcode
     *
     * An empty lnurlCallbackFilter allows every syntactically valid HTTPS
     * callback, preserving the previous behavior.
     */
    ESP32TlsConfig tls;
    NostrLimits limits;
    uint16_t connectTimeoutMs;
    uint16_t readTimeoutMs;
    uint8_t maxProtocolViolations;
    // Empty means every syntactically valid HTTPS callback is allowed.
    std::function<bool(const ParsedUrl &)> lnurlCallbackFilter;
    // Empty accepts any standard Bitcoin BOLT11 network; use "bc", "tb",
    // "tbs" or "bcrt" to bind LNURL invoices to one network.
    NostrString expectedBolt11Currency;

    ESP32TransportConfig()
        : tls(), limits(), connectTimeoutMs(10000), readTimeoutMs(15000), maxProtocolViolations(3),
          lnurlCallbackFilter(), expectedBolt11Currency("") {}
};

class ESP32Transport;
class ESP32Connection : public Connection {
    friend class ESP32Transport;

  public:
    void send(NostrString message) override;
    void disconnect() override;
    void loop() override;
    bool isReady() override;
    void addMessageListener(std::function<void(NostrString)> listener) override;
    void addConnectionStatusListener(std::function<void(ConnectionStatus status)> listener) override;
    WebSocketsClient *getWebsocket() { return &ws; }
    ~ESP32Connection() override;

  protected:
    ESP32Connection(ESP32Transport *transport, NostrString url);

  private:
    ESP32Transport *transport;
    WebSocketsClient ws;
    std::vector<std::function<void(NostrString)>> messageListeners;
    std::vector<std::function<void(ConnectionStatus status)>> connectionListeners;
    NostrString fragmentedMessage;
    bool fragmentedMessageRejected = false;
    bool failed = false;
    uint8_t protocolViolations = 0;
    void noteProtocolViolation(const char *message);
};
class ESP32Transport : public Transport {
    friend class ESP32Connection;
  public:
    void getInvoiceFromLNAddr(NostrString addr, MilliSats amountMSats, NostrString comment, std::function<void(NostrString)> callback);
    Connection *connect(NostrString url) override;
    void close();
    void disconnect(Connection *conn) override;
    ~ESP32Transport() override;
    explicit ESP32Transport(const ESP32TransportConfig &config = ESP32TransportConfig());
    bool isReady() override;
    const ESP32TransportConfig &getConfig() const { return config; }
    bool tlsCertificateValidationActive() const { return config.tls.caCertificatePem != nullptr; }

  private:
    void httpsGet(NostrString url, std::function<void(NostrString)> callback);
    bool configureTls(WiFiClientSecure &client);
    bool allowTlsConnection();
    void noteUnverifiedTls();
    ESP32TransportConfig config;
    bool warnedUnverifiedTls = false;
    WiFiClientSecure httpsClient;
    std::vector<Connection *> connections;
};
} // namespace esp32
} // namespace nostr
#endif
#endif
