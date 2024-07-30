
#define USE_SERIAL Serial1

#include <Arduino.h>
#include <SPI.h>
#include <WebSocketsClient.h>
#include <WiFi.h>

IPAddress dns(8, 8, 8, 8);
WebSocketsClient webSocket;

char ssid[] = "Wokwi-GUEST";
char pass[] = "";

int status = WL_IDLE_STATUS;

void webSocketEvent(WStype_t type, uint8_t *payload, size_t length) {
    switch (type) {
    case WStype_DISCONNECTED:
        Serial.printf("[WSc] Disconnected!\n");
        break;
    case WStype_CONNECTED: {
        Serial.printf("[WSc] Connected to url: %s\n", payload);

        webSocket.sendTXT("Connected");
    } break;
    case WStype_TEXT:
        Serial.printf("[WSc] get text: %s\n", payload);
        break;
    case WStype_BIN:
        Serial.printf("[WSc] get binary length: %u\n", length);
        break;
    }
}

void setup() {
    Serial.begin(115200);
    while (!Serial) {
        Serial.println("Serial not ready");
    }

    if (WiFi.status() == WL_NO_SHIELD) {
        while (true) {
            Serial.println("WiFi shield not present");
        }
    }

    while (status != WL_CONNECTED) {
        Serial.print("Attempting to connect to SSID: ");
        Serial.println(ssid);
        status = WiFi.begin(ssid, pass);
        delay(10000);
    }

    WiFi.setDNS(dns);
    Serial.print("Dns configured.");

    webSocket.beginSSL("echo.websocket.org", 443);
    webSocket.onEvent(webSocketEvent);
}

void loop() {
    webSocket.loop();
}