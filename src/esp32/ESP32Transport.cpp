#include "ESP32Transport.h"
#include "NostrCommon.h"

#if defined(_ESP32_BOARD_) || defined(_ESP8266_BOARD_)
using namespace nostr;

void esp32::ESP32Transport::httpsGet(NostrString url, std::function<void(NostrString)> cb) {
    HTTPClient http;
#ifdef _ESP8266_BOARD_
    WiFiClient wifiClient;
    http.begin(wifiClient, url.c_str());
#else
    http.begin(url.c_str());
#endif
    int httpCode = http.GET();
    if (httpCode <= 0 || httpCode != HTTP_CODE_OK) {
        cb("");
        return;
    }
    NostrString str = http.getString();
    http.end();
    cb(str);
}

void esp32::ESP32Transport::getInvoiceFromLNAddr(NostrString addr, unsigned long long amount, NostrString comment, std::function<void(NostrString)> cb) {
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
        if (!NostrString_equals(status,"null") && !NostrString_equals(status, "OK")) {
            Utils::log("LNURLP status is set but not OK, status is: '" + status + "'");
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
            if (!NostrString_equals(callbackStatus,"null") && !NostrString_equals(callbackStatus, "OK")) {
                Utils::log("LNURLP callback status is set but not OK, status is: '" + callbackStatus + "'");
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

Connection *esp32::ESP32Transport::connect(NostrString url) {
    ESP32Connection *conn = new ESP32Connection(this, url);
    connections.push_back(conn);
    return conn;
}

void esp32::ESP32Transport::close() {
    for (auto conn : connections) {
        conn->disconnect();
        delete conn;
    }
    connections.clear();
}

void esp32::ESP32Transport::disconnect(Connection *conn) {
    for (size_t i = 0; i < connections.size(); i++) {
        if (connections[i] == conn) {
            conn->disconnect();
            delete conn;
            connections.erase(connections.begin() + i);
            return;
        }
    }
}

esp32::ESP32Connection::ESP32Connection(ESP32Transport *transport, NostrString url) {
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
    if (ssl) {
        Utils::log("Connecting to " + host + " : " + port + " with path " + path + " using SSL...");
#ifdef _ESP8266_BOARD_
        ws.beginSSL(host.c_str(), port, path.c_str());
#else
        ws.beginSSL(host, port, path);
#endif
    } else {
        Utils::log("Connecting to " + host + " : " + port + " with path " + path + "...");
        ws.begin(host, port, path);
    }
    ws.setReconnectInterval(5000);
    ws.onEvent([this](WStype_t type, uint8_t *payload, size_t length) {
        switch (type) {
        case WStype_DISCONNECTED:
            Utils::log("ESP32Connection disconnected.");
            for (auto &listener : connectionListeners) {
                try {
                    listener(ConnectionStatus::DISCONNECTED);
                } catch (std::exception &e) {
                    Utils::log(e.what());
                }
            }
            break;
        case WStype_CONNECTED:
            Utils::log("ESP32Connection connected.");
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
            Utils::log("ESP32Connection error.");
            for (auto &listener : connectionListeners) {
                try {
                    listener(ConnectionStatus::ERROR);
                } catch (std::exception &e) {
                    Utils::log(e.what());
                }
            }
            break;
        case WStype_FRAGMENT_TEXT_START:
            Utils::log("Fragmented message start.");
            this->fragmentedMessage = NostrString_fromChars((char *)payload);
            break;
        case WStype_FRAGMENT:
            Utils::log("Fragmented message continued.");
            this->fragmentedMessage += NostrString_fromChars((char *)payload);
            break;
        case WStype_FRAGMENT_FIN:
            Utils::log("Fragmented message finalized.");
            this->fragmentedMessage += NostrString_fromChars((char *)payload);
            Utils::log("Assembled fragmented message: " + this->fragmentedMessage);
            for (auto &listener : messageListeners) {
                try {
                    listener(this->fragmentedMessage);
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

void esp32::ESP32Connection::send(NostrString message) {
    Utils::log("Sending message: " + message);
    ws.sendTXT((uint8_t *)NostrString_toChars(message), message.length());
}

void esp32::ESP32Connection::disconnect() {
    ws.disconnect();
    this->transport->disconnect(this);
}

bool esp32::ESP32Transport::isReady() {
    return (WiFi.status() == WL_CONNECTED);
}

void esp32::ESP32Connection::loop() {
    ws.loop();
}

bool esp32::ESP32Connection::isReady() {
    return ws.isConnected();
}

void esp32::ESP32Connection::addMessageListener(std::function<void(NostrString)> listener) {
    messageListeners.push_back(listener);
}

esp32::ESP32Transport::~ESP32Transport() {
    close();
}

esp32::ESP32Connection::~ESP32Connection() {
    ws.disconnect();
}

esp32::ESP32Transport::ESP32Transport() {}

void esp32::ESP32Connection::addConnectionStatusListener(std::function<void(ConnectionStatus status)> listener) {
    connectionListeners.push_back(listener);
}

#endif