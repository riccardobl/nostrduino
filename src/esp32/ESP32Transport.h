#ifndef _NOSTR_ESP32_TRANSPORT_H
#define _NOSTR_ESP32_TRANSPORT_H 1
#ifdef ESP32

#include "ArduinoJson.h"
#include "NostrString.h"
#include "Transport.h"
#include "Utils.h"
#include "WiFi.h"
#include <HTTPClient.h>
#include <WebSocketsClient.h>
#include <functional>
#include <initializer_list>
#include <vector>
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
    void getInvoiceFromLNAddr(NostrString addr, unsigned long long amountMSats, NostrString comment , std::function<void(NostrString)> callback);
    Connection *connect(NostrString url) override;
    void close();
    void disconnect(Connection *conn) override;
    ~ESP32Transport() override;
    ESP32Transport();
    bool isReady() override;

  private:
    void httpsGet(NostrString url, std::function<void(NostrString)> callback);
    std::vector<Connection *> connections;
};
} 
} 
#endif
#endif
