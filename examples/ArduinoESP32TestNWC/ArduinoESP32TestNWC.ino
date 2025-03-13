// Extensive Nostr Wallet Connect (NWC) example for Nostrduino.
//
// Tested on the Arduino IDE 1.8.13 with the following libraries installed:
// - ArduinoJson (>=7.1.0)
// - uBitcoin (>=0.2.0)
// - WebSockets (>=2.4.1)
// - Nostrduino (>= 1.3.6)
//
// Inspired by nostrduino/examples/ESP32TestNWC/ESP32TestNWC.cpp
#include <NostrCommon.h>

#include <Arduino.h>
#include "esp32/ESP32Platform.h"

#include "services/NWC.h"

// CONFIGURATION
// #define WIFI_SSID "Wokwi-GUEST" // works with Wokwi (closed source)
#define WIFI_SSID "Open Wifi" // works with the ESP32 fork of QEMU (open source) with wifi support (https://github.com/LightningPiggy/qemu)
#define WIFI_PASS ""

#define NWC_URL "nostr+walletconnect://69effe7b49a6dd5cf525bd0905917a5005ffe480b58eeb8e861418cf3ae760d9?relay=wss://relay.getalby.com/v1&secret=90a44d69283f46b0c917925980c9ade65449a5d85839162387948b42cd58bb1f&lud16=rblb@getalby.com"

// Note: running this setting with default values will send 10 sats to the address below
#define PAYOUT_ADDRESS "thomas@demo.lnpiggy.com"
#define PAYOUT_AMOUNT_MSAT 10000

unsigned long long listTransactionsUntilTime = 0; // if listTransactionsUntil is 0, then it will default to "now", so it will fetch the latest transaction
unsigned long long requestedListTransactionsUntilTime;

nostr::NWC *nwc;
nostr::Transport *transport;

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
    nostr::esp32::ESP32Platform::initWifi(WIFI_SSID, WIFI_PASS);

    Serial.println("Init time");
    nostr::esp32::ESP32Platform::initTime("pool.ntp.org");

    Serial.println("Init Nostr");
    nostr::esp32::ESP32Platform::initNostr(true);

    Serial.println("Ready!");

    ////////////////////////
    /// END OF INITIALIZATION
    ////////////////////////

    testNWC(); // queue up a few NWC calls, even though they won't be executed as long as nwc->loop(); isn't called
    subscribeNWC(); // subscribe to outgoing and incoming payment notifications
    listTransaction(); // list one transaction (
}


void loop() {
    nwc->loop();

/*
   // Uncomment to list all transactions one-by-one:
    if (listTransactionsUntilTime != requestedListTransactionsUntilTime) getNextTransaction(); // get next transaction when the previous one is done
*/

}

void testNWC() {
    try {
        transport = nostr::esp32::ESP32Platform::getTransport();
        nwc = new nostr::NWC(transport, NWC_URL);

        nwc->getBalance([&](nostr::GetBalanceResponse resp) { Serial.println("[!] Balance: " + String(resp.balance) + " msatoshis"); },
                        [](String err, String errMsg) { Serial.println("[!] Error: " + err + " " + errMsg); });
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
                Serial.println("[!] Notifications: ");
                for (auto notification : resp.notifications) {
                    Serial.println("  " + notification);
                }
            },
            [](String err, String errMsg) { Serial.println("[!] Error: " + err + " " + errMsg); });

        transport->getInvoiceFromLNAddr(PAYOUT_ADDRESS, PAYOUT_AMOUNT_MSAT, "Arduino NWC test", [&](String invoice) {
            if (NostrString_equals(invoice, "")) {
                Serial.println("[!] Error: Could not get invoice from LN address");
            }
            Serial.println("[!] Paying " + String(PAYOUT_AMOUNT_MSAT) + " msats to " + PAYOUT_ADDRESS + " invoice: " + invoice);
            nwc->payInvoice(
                invoice, PAYOUT_AMOUNT_MSAT, [&](nostr::PayInvoiceResponse resp) { Serial.println("[!] Payment successful"); },
                [](String err, String errMsg) { Serial.println("[!] Error: " + err + " " + errMsg); });
        });

    } catch (std::exception &e) {
        Serial.println("[!] Exception: " + String(e.what()));
    }
}

void subscribeNWC() {
  Serial.println("Listening for payment notifications...");
     nwc->subscribeNotifications(
        [](nostr::NotificationResponse res) {
            nostr::Nip47Notification notification = res.notification;
            Serial.println("Notification Type: " + notification.notificationType);
            Serial.println("Type: " + notification.type);
            Serial.println("Amount: " + NostrString_fromUInt(notification.amount));
            Serial.println("Payment Hash: " + notification.paymentHash);
            Serial.println("Description: " + notification.description);
            Serial.println("Description Hash: " + notification.descriptionHash);
            Serial.println("Invoice: " + notification.invoice);
            Serial.println("Preimage: " + notification.preimage);
            Serial.println("Fees Paid: " + NostrString_fromUInt(notification.feesPaid));
            Serial.println("Created At: " + NostrString_fromUInt(notification.createdAt));
            Serial.println("Settled At: " + NostrString_fromUInt(notification.settledAt));
        },
        [](NostrString errorCode, NostrString errorMessage) {
            Serial.println("Notification Error: " + errorCode + " - " + errorMessage);
        }
    );
}

// This fetches one transaction, because stack memory is limited on ESP32.
// More than one is also possible, but to really list all transactions,
// a loop that increments the offset or decrements the listTransactionsUntilTime is needed.
void listTransaction() {
  Serial.println("Doing listTransactions from until listTransactionsUntilTime " + String(listTransactionsUntilTime) + ":");
  requestedListTransactionsUntilTime = listTransactionsUntilTime;
  try {

    nwc->listTransactions(0,listTransactionsUntilTime,1,0,false,"incoming", [&](nostr::ListTransactionsResponse resp) {
        Serial.println("[!] Received listTransactions result from until listTransactionsUntilTime " + String(listTransactionsUntilTime) + ":");
        for (auto transaction : resp.transactions) {
          Serial.println("=> Got transaction: " + String(transaction.amount) + " msat createdAt: " + String(transaction.createdAt) + " with description: '" + transaction.description + "'");
          listTransactionsUntilTime = transaction.createdAt-1; // prepare for the next listTransactions() call by lowering the "until" time
        }
    },
    [](String err, String errMsg) { Serial.println("[!] Error: " + err + " " + errMsg); });

  } catch (std::exception &e) {
      Serial.println("[!] Exception: " + String(e.what()));
  }
}
