#ifndef _NOSTR_ESP32_TRANSPORT_H
#define _NOSTR_ESP32_TRANSPORT_H 1
#include <HTTPClient.h>
#include <WebSocketsClient.h>

#include "ArduinoJson.h"
#include "NostrString.h"
#include "Transport.h"
#include "Utils.h"
#include "WiFi.h"
namespace nostr {
namespace esp32 {
class ESP32Transport;
class ESP32Connection : public Connection {
    friend class ESP32Transport;

  public:
    void send(NostrString message) override;
    void disconnect() override;
    void loop() override;
    bool isReady() override;
    void addMessageListener(std::function<void(NostrString)> listener) override;
    ~ESP32Connection() override;

  protected:
    ESP32Connection(ESP32Transport *transport, NostrString url);

  private:
    ESP32Transport *transport;
    WebSocketsClient ws;
    std::vector<std::function<void(NostrString)>> messageListeners;
};
class ESP32Transport : public Transport {
  public:
    NostrString getInvoiceFromLNAddr(NostrString addr, unsigned long long amount, NostrString comment = "") override;
    Connection *connect(NostrString url) override;
    void close();
    void disconnect(Connection *conn) override;
    ~ESP32Transport() override;
    ESP32Transport();
    bool isReady() override;

  private:
    std::vector<Connection *> connections;
};
} 
} 

#endif
