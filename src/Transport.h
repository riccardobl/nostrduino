#ifndef _NOSTR_TRANSPORT_H
#define _NOSTR_TRANSPORT_H
#include <Arduino.h>
#include "NostrString.h"
#include <functional>

namespace nostr {
enum class ConnectionStatus { CONNECTED, DISCONNECTED, ERROR };
class Connection {
  public:
    virtual void addMessageListener(std::function<void(NostrString)> listener) = 0;
    virtual void send(NostrString message) = 0;
    virtual void disconnect() = 0;
    virtual ~Connection() = default;
    Connection() = default;
    virtual void loop() = 0;
    virtual bool isReady() = 0;
    virtual void addConnectionStatusListener(std::function<void(ConnectionStatus status)> listener) = 0;
};
class Transport {
  public:
    virtual void getInvoiceFromLNAddr(NostrString addr, unsigned long long amountMSats, NostrString comment, std::function<void(NostrString)> callback) = 0;
    virtual Connection *connect(NostrString url) = 0;
    virtual ~Transport() = default;
    Transport() = default;
    virtual void disconnect(Connection *conn) = 0;
    virtual bool isReady() = 0;
    virtual void close() = 0;
    virtual void loop() {};
};
} // namespace nostr
#endif