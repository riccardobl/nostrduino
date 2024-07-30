#include "UNOR4Transport.h"

#ifdef _UNOR4_BOARD_

using namespace nostr;
void unor4::UNOR4Transport::loop() {
    for (int i = 0; i < getRequests.size(); i++) {
        ActiveRequest req = getRequests[i];
        while (req.client->available()) {
            char c = req.client->read();
            req.content += c;
        }
        if (!req.client->connected()) {
            NostrString httpBody = NostrString_substring(req.content, NostrString_indexOf(req.content, "\r\n\r\n") + 4);
            req.cb(httpBody);
            req.client->stop();
            delete req.client;
            getRequests.erase(getRequests.begin() + i);
            i--;
        }
    }
}
void unor4::UNOR4Transport::httpsGet(NostrString url, std::function<void(NostrString)> cb) {
    NostrString host = NostrString_substring(url, 8);
    host = NostrString_substring(host, 0, NostrString_indexOf(host, "/"));
    NostrString path = NostrString_substring(url, NostrString_indexOf(url, "/"));

    WiFiSSLClient *client = new WiFiSSLClient();
    if (client->connect(url.c_str(), 443)) {
        client->println("GET " + path + " HTTP/1.1");
        client->println("Host: " + host);
        client->println("Connection: close");
        client->println();
    }
    ActiveRequest req;
    req.client = client;
    req.cb = cb;
    getRequests.push_back(req);
}
void unor4::UNOR4Transport::getInvoiceFromLNAddr(NostrString addr, unsigned long long amount, NostrString comment, std::function<void(NostrString)> cb) {
    // username@domain.com
    // becomes https://domain.com/.well-known/lnurlp/username
    int atpos = NostrString_indexOf(addr, "@");
    if (atpos == -1) {
        cb("");
        return;
    }
    NostrString username = NostrString_substring(addr, 0, atpos);
    NostrString domain = NostrString_substring(addr, atpos + 1);
    NostrString url = "https://" + domain + "/.well-known/lnurlp/" + NostrString_urlEncode(username);

    this->httpsGet(url, [this, amount, comment, cb](NostrString json) {
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, json);
        if (error) {
            Utils::log("Failed to parse lnaddr JSON " + json);
            cb("");
            return;
        }
        NostrString status = doc["status"];
        if (!NostrString_equals(status, "OK")) {
            Utils::log("LNURLP status not OK");
            cb("");
            return;
        }
        NostrString callback = doc["callback"];
        if (NostrString_length(callback) == 0) {
            Utils::log("LNURLP callback not found");
            cb("");
            return;
        }
        NostrString lnurlp = callback;
        if (NostrString_indexOf(lnurlp, "?") != -1) {
            lnurlp += "&";
        } else {
            lnurlp += "?";
        }
        lnurlp += "amount=" + NostrString_intToString(amount);
        if (NostrString_length(comment) > 0) {
            lnurlp += "&comment=" + NostrString_urlEncode(comment);
        }
        doc.clear();
        this->httpsGet(lnurlp, [cb](NostrString json) {
            JsonDocument doc;

            DeserializationError error = deserializeJson(doc, json);
            if (error) {
                Utils::log("Failed to parse lnurlp JSON " + json);
                cb("");
                return;
            }
            NostrString callbackStatus = doc["status"];
            if (!NostrString_equals(callbackStatus, "OK")) {
                Utils::log("LNURLP callback status not OK");
                cb("");
                return;
            }
            NostrString invoice = doc["pr"];
            if (NostrString_length(invoice) == 0) {
                Utils::log("LNURLP invoice not found");
                cb("");
                return;
            }
            cb(invoice);
        });
    });
}

Connection *unor4::UNOR4Transport::connect(NostrString url) {
    UNOR4Connection *conn = new UNOR4Connection(this, url);
    connections.push_back(conn);
    return conn;
}

void unor4::UNOR4Transport::close() {
    for (auto conn : connections) {
        conn->disconnect();
        delete conn;
    }
    for (int i = 0; i < getRequests.size(); i++) {
        getRequests[i].client->stop();
        delete getRequests[i].client;
    }
    getRequests.clear();
    connections.clear();
}

void unor4::UNOR4Transport::disconnect(Connection *conn) {
    for (size_t i = 0; i < connections.size(); i++) {
        if (connections[i] == conn) {
            conn->disconnect();
            delete conn;
            connections.erase(connections.begin() + i);
            return;
        }
    }
}

unor4::UNOR4Connection::UNOR4Connection(UNOR4Transport *transport, NostrString url) {
    this->transport = transport;
    bool ssl = NostrString_startsWith(url, "wss://");
    url = NostrString_substring(url, ssl ? 6 : 5);
    NostrString host = NostrString_substring(url, 0, NostrString_indexOf(url, "/"));
    NostrString path = NostrString_substring(url, NostrString_indexOf(url, "/"));
    if (NostrString_equals(path, ""))
        path = "/";
    int port = ssl ? 443 : 80;
    if (NostrString_indexOf(host, ":") != -1) {
        NostrString portStr = NostrString_substring(host, NostrString_indexOf(host, ":") + 1);
        port = NostrString_toInt(portStr);
        host = NostrString_substring(host, 0, NostrString_indexOf(host, ":"));
    }
    // if (ssl) {
    //     Utils::log("Connecting to " + host + " : " + port + " with path " + path + " using SSL...");
    //     ws.beginSSL(host, port, path);
    // } else {
    Utils::log("Connecting to " + host + " : " + port + " with path " + path + "...");
    ws.begin(host, port, path);
    // }
    ws.setReconnectInterval(5000);
    ws.onEvent([this](WStype_t type, uint8_t *payload, size_t length) {
        switch (type) {
        case WStype_DISCONNECTED:
            Utils::log("UNOR4Connection disconnected.");
            for (auto &listener : connectionListeners) {
                try {
                    listener(ConnectionStatus::DISCONNECTED);
                } catch (std::exception &e) {
                    Utils::log(e.what());
                }
            }
            break;
        case WStype_CONNECTED:
            Utils::log("UNOR4Connection connected.");
            for (auto &listener : connectionListeners) {
                try {
                    listener(ConnectionStatus::CONNECTED);
                } catch (std::exception &e) {
                    Utils::log(e.what());
                }
            }
            break;
        case WStype_TEXT: {
            NostrString message = NostrString_fromChars((char *)payload);
            Utils::log("Received message: " + message);
            for (auto &listener : messageListeners) {
                try {
                    listener(message);
                } catch (std::exception &e) {
                    Utils::log(e.what());
                }
            }

            break;
        }
        case WStype_ERROR:
            Utils::log("UNOR4Connection error.");
            for (auto &listener : connectionListeners) {
                try {
                    listener(ConnectionStatus::ERROR);
                } catch (std::exception &e) {
                    Utils::log(e.what());
                }
            }
            break;
        default:
            break;
        }
    });
}

void unor4::UNOR4Connection::send(NostrString message) {
    Utils::log("Sending message: " + message);
    ws.sendTXT((uint8_t *)NostrString_toChars(message), message.length());
}

void unor4::UNOR4Connection::disconnect() {
    ws.disconnect();
    this->transport->disconnect(this);
}

bool unor4::UNOR4Transport::isReady() {
    return (WiFi.status() == WL_CONNECTED);
}

void unor4::UNOR4Connection::loop() {
    ws.loop();
}

bool unor4::UNOR4Connection::isReady() {
    return ws.isConnected();
}

void unor4::UNOR4Connection::addMessageListener(std::function<void(NostrString)> listener) {
    messageListeners.push_back(listener);
}

unor4::UNOR4Transport::~UNOR4Transport() {
    close();
}

unor4::UNOR4Connection::~UNOR4Connection() {
    ws.disconnect();
}

unor4::UNOR4Transport::UNOR4Transport() {}
void unor4::UNOR4Connection::addConnectionStatusListener(std::function<void(ConnectionStatus status)> listener) {
    connectionListeners.push_back(listener);
}

#endif