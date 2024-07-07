
#include "services/NWC.h"
#include "Transport.h"
#include "esp32/ESP32Platform.h"


// CONFIGURATION
#define WIFI_SSID "Wokwi-GUEST"
#define WIFI_PASS ""
#define WIFI_CHANNEL 6
// IMPORTANT !!! Set a valid NWC url here
#define NWC_URL "nostr+walletconnect://69effe7b49a6dd5cf525bd0905917a5005ffe480b58eeb8e861418cf3ae760d9?relay=wss://relay.getalby.com/v1&secret=1205ebd25d79a52a4e51920f6e1981a43400dc49daaa65d367bd8090218d206c&lud16=rblb@getalby.com"
// Note: running this setting with default values will send 10 sats to the address below
#define PAYOUT_ADDRESS "zap@rblb.it"
#define PAYOUT_AMOUNT_MSAT 10000


void testNWC();

void setup() {
    ////////////////////////
    /// INITIALIZATION
    ///   Note: you need some form of this code in your sketch
    ///         or the library will not work properly.
    ///         If you don't know what to do, just copy this code.
    ///         If you are initializing time and wifi somewhere else in your
    ///         code, you can just omit the related lines here and call only
    ///         initNostr
    ////////////////////////
    Serial.begin(115200);

    Serial.println("Init wifi");
    nostr::esp32::ESP32Platform::initWifi(WIFI_SSID, WIFI_PASS, WIFI_CHANNEL);

    Serial.println("Init time");
    nostr::esp32::ESP32Platform::initTime("pool.ntp.org");

    Serial.println("Init Nostr");
    nostr::esp32::ESP32Platform::initNostr(true);

    Serial.println("Ready!");

    ////////////////////////
    /// END OF INITIALIZATION
    ////////////////////////

    testNWC();
}
nostr::NWC *nwc;
nostr::Transport *transport;

void loop() {
    nwc->loop();
}

void testNWC() {
    transport = nostr::esp32::ESP32Platform::getTransport();
    nwc = new nostr::NWC(transport, NWC_URL);

    nwc->getBalance(
        [&](nostr::GetBalanceResponse resp) {
            Serial.println("[!] Balance: " + String(resp.balance) + " msatoshis");
        },
        [](String err, String errMsg) {
            Serial.println("[!] Error: " + err + " " + errMsg);
        });
    nwc->getInfo(
        [&](nostr::GetInfoResponse resp) {
            Serial.println("[!] Alias: " + resp.alias);
            Serial.println("[!] Color: " + resp.color);
            Serial.println("[!] Pubkey: " + resp.pubkey);
            Serial.println("[!] Network: " + resp.network);
            Serial.println("[!] Block height: " + String(resp.blockHeight));
            Serial.println("[!] Block hash: " + resp.blockHash);
            Serial.println("[!] Methods: ");
            for (auto method : resp.methods) {
                Serial.println("  " + method);
            }
        },
        [](String err, String errMsg) {
            Serial.println("[!] Error: " + err + " " + errMsg);
        });

    NostrString invoice = transport->getInvoiceFromLNAddr( PAYOUT_ADDRESS, PAYOUT_AMOUNT_MSAT, "Arduino NWC test");
    Serial.println("[!] Paying " + String(PAYOUT_AMOUNT_MSAT) + " msats to " +
                   PAYOUT_ADDRESS + " invoice: " + invoice);
    nwc->payInvoice(
        invoice, PAYOUT_AMOUNT_MSAT,
        [&](nostr::PayInvoiceResponse resp) {
            Serial.println("[!] Payment successful");
        },
        [](String err, String errMsg) {
            Serial.println("[!] Error: " + err + " " + errMsg);
        });
}