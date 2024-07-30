#include "NostrCommon.h"
#ifndef _NOSTR_UNOR4_TRANSPORT_H
#define _NOSTR_UNOR4_TRANSPORT_H
#ifdef _UNOR4_BOARD_

#include <WebSocketsClient.h>

#include "ArduinoJson.h"
#include "IPAddress.h"
#include "NostrString.h"
#include "NostrTransport.h"
#include "NostrUtils.h"
#include "WiFiS3.h"
#include "WiFiSSLClient.h"
#include <initializer_list>
#include <vector>

namespace nostr {
namespace unor4 {

typedef struct m_ActiveRequest {
    WiFiSSLClient *client;
    String content;
    std::function<void(NostrString)> cb;
} ActiveRequest;

class UNOR4Transport;
class UNOR4Connection : public Connection {
    friend class UNOR4Transport;

  public:
    void send(NostrString message) override;
    void disconnect() override;
    void loop() override;
    bool isReady() override;
    void addMessageListener(std::function<void(NostrString)> listener) override;
    ~UNOR4Connection() override;
    void addConnectionStatusListener(std::function<void(ConnectionStatus status)> listener) override;

  protected:
    UNOR4Connection(UNOR4Transport *transport, NostrString url);

  private:
    UNOR4Transport *transport;
    WebSocketsClient ws;
    std::vector<std::function<void(NostrString)>> messageListeners;
    std::vector<std::function<void(ConnectionStatus status)>> connectionListeners;
};
class UNOR4Transport : public Transport {
  public:
    void getInvoiceFromLNAddr(NostrString addr, unsigned long long amountMSats, NostrString comment, std::function<void(NostrString)> callback);
    Connection *connect(NostrString url) override;
    void close();
    void disconnect(Connection *conn) override;
    ~UNOR4Transport() override;
    UNOR4Transport();
    bool isReady() override;
    void loop() override;

  private:
    void httpsGet(NostrString url, std::function<void(NostrString)> callback);
    std::vector<Connection *> connections;
    std::vector<ActiveRequest> getRequests;
};
} // namespace unor4
} // namespace nostr

#endif
#endif