#include "Common.h"
#ifndef _NOSTR_PICOW_TRANSPORT_H
#define _NOSTR_PICOW_TRANSPORT_H
#ifdef _PICOW_BOARD_

#include <WebSocketsClient.h>

#include "ArduinoJson.h"
#include "IPAddress.h"
#include "NostrString.h"
#include "Transport.h"
#include "Utils.h"
#include "HTTPClient.h"
#include <initializer_list>
#include <vector>
namespace nostr {
namespace picow {


class PICOWTransport;
class PICOWConnection : public Connection {
    friend class PICOWTransport;

  public:
    void send(NostrString message) override;
    void disconnect() override;
    void loop() override;
    bool isReady() override;
    void addMessageListener(std::function<void(NostrString)> listener) override;
    ~PICOWConnection() override;

  protected:
    PICOWConnection(PICOWTransport *transport, NostrString url);

  private:
    PICOWTransport *transport;
    WebSocketsClient ws;
    std::vector<std::function<void(NostrString)>> messageListeners;
    
};
class PICOWTransport : public Transport {
  public:
    void getInvoiceFromLNAddr(NostrString addr, unsigned long long amountMSats, NostrString comment, std::function<void(NostrString)> callback);
    Connection *connect(NostrString url) override;
    void close();
    void disconnect(Connection *conn) override;
    ~PICOWTransport() override;
    PICOWTransport();
    bool isReady() override;
  private:
    void httpsGet(NostrString url, std::function<void(NostrString)> callback);
    std::vector<Connection *> connections;
};
} 
} 

#endif
#endif