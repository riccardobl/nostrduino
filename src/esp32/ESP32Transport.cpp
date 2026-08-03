#include "ESP32Transport.h"
#include "NostrCommon.h"
#include "NostrUrl.h"
#include <algorithm>

#if defined(_ESP32_BOARD_)
using namespace nostr;

void esp32::ESP32Transport::httpsGet(NostrString url, std::function<void(NostrString)> cb) {
    ParsedUrl parsedUrl;
    if (NostrString_length(url) < 0 || static_cast<size_t>(NostrString_length(url)) > config.limits.maxUrlBytes ||
        !parseHttpsUrl(url, parsedUrl) || !allowTlsConnection()) {
        cb("");
        return;
    }
    if (!configureTls(httpsClient)) {
        cb("");
        return;
    }
    HTTPClient http;
    if (!http.begin(httpsClient, url.c_str())) {
        cb("");
        return;
    }
    http.setConnectTimeout(config.connectTimeoutMs);
    http.setTimeout(config.readTimeoutMs);
    http.setFollowRedirects(HTTPC_DISABLE_FOLLOW_REDIRECTS);
    int httpCode = http.GET();
    if (httpCode <= 0 || httpCode != HTTP_CODE_OK) {
        http.end();
        cb("");
        return;
    }
    const int responseSize = http.getSize();
    if (responseSize >= 0 && static_cast<size_t>(responseSize) > config.limits.maxHttpResponseBytes) {
        http.end();
        cb("");
        return;
    }
    WiFiClient *stream = http.getStreamPtr();
    NostrString str;
    if (stream == nullptr) {
        http.end();
        cb("");
        return;
    }
    if (responseSize > 0 && !str.reserve(static_cast<unsigned int>(responseSize))) {
        http.end();
        cb("");
        return;
    }
    uint8_t buffer[512];
    uint32_t lastProgress = millis();
    while (http.connected() || stream->available()) {
        const size_t available = stream->available();
        if (available == 0) {
            if (static_cast<uint32_t>(millis() - lastProgress) >= config.readTimeoutMs) {
                str = "";
                break;
            }
            delay(1);
            continue;
        }
        const size_t toRead = available < sizeof(buffer) ? available : sizeof(buffer);
        const int readSize = stream->readBytes(reinterpret_cast<char *>(buffer), toRead);
        if (readSize <= 0 || static_cast<size_t>(readSize) > config.limits.maxHttpResponseBytes ||
            str.length() > config.limits.maxHttpResponseBytes - static_cast<size_t>(readSize)) {
            str = "";
            break;
        }
        if (!NostrString_appendBytes(str, reinterpret_cast<const char *>(buffer),
                                    static_cast<size_t>(readSize), config.limits.maxHttpResponseBytes)) {
            str = "";
            break;
        }
        lastProgress = millis();
    }
    for (size_t i = 0; i < sizeof(buffer); ++i) buffer[i] = 0;
    http.end();
    cb(str);
}

void esp32::ESP32Transport::getInvoiceFromLNAddr(NostrString addr, MilliSats amount, NostrString comment, std::function<void(NostrString)> cb) {
    if (!isRequiredMilliSatsValid(amount)) {
        if (cb) cb("");
        return;
    }
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
        lnurlp += "amount=" + NostrString_fromMilliSats(amount);
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
    ParsedUrl parsedUrl;
    if (NostrString_length(url) < 0 || static_cast<size_t>(NostrString_length(url)) > transport->config.limits.maxUrlBytes ||
        !parseWebSocketUrl(url, parsedUrl)) {
        failed = true;
        Utils::log("Rejected invalid WebSocket URL");
        return;
    }
    if (parsedUrl.secure) {
        Utils::log("Connecting to " + parsedUrl.host + " : " + parsedUrl.port + " with path " + parsedUrl.path + " using SSL...");
        if (!transport->allowTlsConnection()) {
            failed = true;
            return;
        }
        if (transport->config.tls.caCertificatePem != nullptr)
            ws.beginSslWithCA(parsedUrl.host.c_str(), parsedUrl.port, parsedUrl.path.c_str(), transport->config.tls.caCertificatePem);
        else {
            transport->noteUnverifiedTls();
            ws.beginSSL(parsedUrl.host.c_str(), parsedUrl.port, parsedUrl.path.c_str());
        }
    } else {
        Utils::log("Connecting to " + parsedUrl.host + " : " + parsedUrl.port + " with path " + parsedUrl.path + "...");
        ws.begin(parsedUrl.host, parsedUrl.port, parsedUrl.path);
    }
    ws.setReconnectInterval(5000);
    ws.onEvent([this](WStype_t type, uint8_t *payload, size_t length) {
        switch (type) {
        case WStype_DISCONNECTED:
            fragmentedMessage = "";
            fragmentedMessageRejected = false;
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
            fragmentedMessage = "";
            fragmentedMessageRejected = false;
            protocolViolations = 0;
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
            fragmentedMessage = "";
            fragmentedMessageRejected = false;
            if (payload == nullptr || length > this->transport->config.limits.maxWebSocketMessageBytes) {
                noteProtocolViolation("Rejected oversized or malformed WebSocket text message");
                break;
            }
            NostrString message = NostrString_fromBytes(reinterpret_cast<const char *>(payload), length);
            if (NostrString_length(message) < 0 || static_cast<size_t>(NostrString_length(message)) != length) {
                noteProtocolViolation("Failed to allocate WebSocket text message");
                break;
            }
            Utils::log("Received WebSocket text message");
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
            fragmentedMessage = "";
            fragmentedMessageRejected = false;
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
            fragmentedMessage = "";
            fragmentedMessageRejected = false;
            Utils::log("Fragmented message start.");
            if (payload == nullptr || !NostrString_appendBytes(this->fragmentedMessage,
                    reinterpret_cast<const char *>(payload), length,
                    this->transport->config.limits.maxWebSocketMessageBytes)) {
                fragmentedMessage = "";
                fragmentedMessageRejected = true;
                noteProtocolViolation("Rejected oversized or malformed WebSocket message fragment");
            }
            break;
        case WStype_FRAGMENT:
            Utils::log("Fragmented message continued.");
            if (!fragmentedMessageRejected && (payload == nullptr || !NostrString_appendBytes(this->fragmentedMessage,
                    reinterpret_cast<const char *>(payload), length,
                    this->transport->config.limits.maxWebSocketMessageBytes))) {
                fragmentedMessage = "";
                fragmentedMessageRejected = true;
                noteProtocolViolation("Rejected oversized or malformed WebSocket message fragment");
            }
            break;
        case WStype_FRAGMENT_FIN:
            Utils::log("Fragmented message finalized.");
            if (!fragmentedMessageRejected && (payload == nullptr || !NostrString_appendBytes(this->fragmentedMessage,
                    reinterpret_cast<const char *>(payload), length,
                    this->transport->config.limits.maxWebSocketMessageBytes))) {
                fragmentedMessageRejected = true;
                noteProtocolViolation("Rejected oversized or malformed WebSocket message fragment");
            }
            if (!fragmentedMessageRejected) {
                for (auto &listener : messageListeners) {
                    try {
                        listener(this->fragmentedMessage);
                    } catch (std::exception &e) {
                        Utils::log(e.what());
                    }
                }
            }
            fragmentedMessage = "";
            fragmentedMessageRejected = false;
            break;
        default:
            break;
        }
    });
}

void esp32::ESP32Connection::send(NostrString message) {
    if (failed || NostrString_length(message) < 0 || static_cast<size_t>(NostrString_length(message)) > transport->config.limits.maxWebSocketMessageBytes) {
        Utils::log("Rejected oversized outgoing WebSocket message");
        return;
    }
    Utils::log("Sending WebSocket message");
    ws.sendTXT(message);
}

void esp32::ESP32Connection::disconnect() {
    ws.disconnect();
}

bool esp32::ESP32Transport::isReady() {
    return (WiFi.status() == WL_CONNECTED);
}

void esp32::ESP32Connection::loop() {
    ws.loop();
}

bool esp32::ESP32Connection::isReady() {
    return !failed && ws.isConnected();
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

esp32::ESP32Transport::ESP32Transport(const ESP32TransportConfig &config) : config(config) {}

bool esp32::ESP32Transport::configureTls(WiFiClientSecure &client) {
    if (!allowTlsConnection()) return false;
    client.setTimeout(config.readTimeoutMs);
    if (config.tls.caCertificatePem != nullptr) {
        client.setCACert(config.tls.caCertificatePem);
    } else {
        noteUnverifiedTls();
        client.setInsecure();
    }
    return true;
}

bool esp32::ESP32Transport::allowTlsConnection() {
    if (config.tls.requireCertificateValidation && config.tls.caCertificatePem == nullptr) {
        if (!warnedUnverifiedTls) {
            Utils::log("TLS validation is required but no CA certificate was configured");
            warnedUnverifiedTls = true;
        }
        return false;
    }
    return true;
}

void esp32::ESP32Transport::noteUnverifiedTls() {
    if (!warnedUnverifiedTls) {
        Utils::log("TLS certificate validation is disabled for this transport");
        warnedUnverifiedTls = true;
    }
}

void esp32::ESP32Connection::addConnectionStatusListener(std::function<void(ConnectionStatus status)> listener) {
    connectionListeners.push_back(listener);
    if (failed && listener) {
        try {
            listener(ConnectionStatus::ERROR);
        } catch (const std::exception &error) {
            Utils::log(error.what());
        }
    }
}

void esp32::ESP32Connection::noteProtocolViolation(const char *message) {
    Utils::log(message);
    if (protocolViolations < UINT8_MAX) ++protocolViolations;
    const uint8_t threshold = transport->config.maxProtocolViolations;
    if (threshold != 0 && protocolViolations >= threshold) ws.disconnect();
}

#endif
