#ifndef _NOSTR_NWC_H
#define _NOSTR_NWC_H
#include <functional>
#include <memory>

#include "ArduinoJson.h"
#include "Nip47.h"
#include "NostrEvent.h"
#include "NostrPool.h"
#include "NostrString.h"
#include "NostrTransport.h"
#include "NostrUtils.h"

#define NWC_RESPONSE_KIND 23195
#define NWC_NOTIFICATION_KIND 23196

#define NWC_DEFAULT_TIMEOUT (60 * 10) // 10 minutes in seconds
#define NWC_INFINITE_TIMEOUT (0xFFFFFFFF) // Max unsigned int, ~136 years

namespace nostr {
class NWCResponseCallbackBase {
  public:
    virtual ~NWCResponseCallbackBase() = default;
    virtual void call(Nip47 *nip47, SignedNostrEvent *ev) {}
    std::function<void(NostrString, NostrString)> onErr = nullptr;
    unsigned long long timestampSeconds;
    unsigned int timeoutSeconds = NWC_DEFAULT_TIMEOUT;
    NostrString eventId;
    NostrString subId;
    unsigned int n = 1;
};

template <typename T> class NWCResponseCallback : public NWCResponseCallbackBase {
  public:
    void call(Nip47 *nip47, SignedNostrEvent *ev) override {
        Nip47Response<T> resp;
        nip47->parseResponse(ev, resp);
        if (NostrString_length(resp.errorCode) > 0) {
            if (this->onErr != nullptr) {
                this->onErr(resp.errorCode, resp.errorMessage);
            }
        } else {
            if (this->onRes != nullptr) {
                this->onRes(resp.result);
            }
        }
    }
    std::function<void(T)> onRes = nullptr;
};

/**
 * A class to interact with the Nostr Wallet Connect services without dealing with the underlying transport
 */
class NWC {
  public:
    ~NWC();
    /**
     * Initialize a standalone NWC service
     * @param transport A transport object, you can get one for the esp32 platform using nostr::esp32::ESP32Platform::getTransport() , for other platforms you need to implement the Transport interface
     * @param nwcUrl The NWC connection URL
     */
    NWC(Transport *transport, NostrString nwcUrl);

    /**
     * Tick the main loop. This should be called in the loop() function of your sketch
     */
    void loop();

    /**
     * Close the service, after this you should throw away the NWC object
     */
    void close();

    /**
     * Pay a lightning invoice
     * @param invoice The invoice to pay
     * @param amount The amount to pay, if not specified the full amount will be paid (optional)
     * @param onRes A callback that will be called when the payment is successful (optional)
     * @param onErr A callback that will be called when the payment fails (optional)
     */
    void payInvoice(NostrString invoice, unsigned long amount = static_cast<unsigned long>(-1), std::function<void(PayInvoiceResponse)> onRes = nullptr,
                    std::function<void(NostrString, NostrString)> onErr = nullptr);

    /**
     * Pay multiple lightning invoices
     * @param invoices A list of invoices to pay
     * @param onRes A callback that will be called when the payment is successful (nb. it will be called once for each invoice) (optional)
     * @param onErr A callback that will be called when the payment fails (optional)
     */
    void multiPayInvoice(std::initializer_list<Invoice> invoices, std::function<void(MultiPayInvoiceResponse)> onRes = nullptr, std::function<void(NostrString, NostrString)> onErr = nullptr);

    /**
     * Send a keysend payment
     * @param pubkey The pubkey to send to
     * @param amount The amount to send
     * @param preimage The preimage to use (optional)
     * @param tlv A list of TLV records to include (optional)
     * @param onRes A callback that will be called when the payment is successful (optional)
     * @param onErr A callback that will be called when the payment fails (optional) 
     */
    void payKeySend(NostrString pubkey, unsigned long amount, NostrString preimage = "", std::initializer_list<TLVRecords> tlv = {}, std::function<void(PayKeySendResponse)> onRes = nullptr,
                    std::function<void(NostrString, NostrString)> onErr = nullptr);

    /**
     * Send multiple keysend payments
     * @param keySends A list of keysend payments to send
     * @param onRes A callback that will be called when the payment is successful (nb. it will be called once for each keysend) (optional)
     * @param onErr A callback that will be called when the payment fails (optional)
     */
    void multiPayKeySend(std::initializer_list<KeySend> keySends, std::function<void(MultiPayKeySendResponse)> onRes = nullptr, std::function<void(NostrString, NostrString)> onErr = nullptr);

    /**
     * Create a lightning invoice
     * @param amount The amount to invoice
     * @param description A description of the invoice (optional)
     * @param descriptionHash A hash of the description (optional)
     * @param expiry The expiry time of the invoice in seconds (optional)
     * @param onRes A callback that will be called when the invoice is created (optional)
     * @param onErr A callback that will be called when the invoice creation fails (optional)
     */
    void makeInvoice(unsigned long amount, NostrString description = "", NostrString descriptionHash = "", unsigned long expiry = 0, std::function<void(MakeInvoiceResponse)> onRes = nullptr,
                     std::function<void(NostrString, NostrString)> onErr = nullptr);

    /**
     * Look up a payment hash
     * @param paymentHash The payment hash to look up
     * @param onRes A callback that will be called when the payment is found (optional)
     * @param onErr A callback that will be called when the payment is not found (optional)
     */
    void lookUpPaymentHash(NostrString paymentHash, std::function<void(LookUpInvoiceResponse)> onRes = nullptr, std::function<void(NostrString, NostrString)> onErr = nullptr);

    /**
     * Look up an invoice
     * @param invoice The invoice to look up
     * @param onRes A callback that will be called when the invoice is found (optional)
     * @param onErr A callback that will be called when the invoice is not found (optional)
     */
    void lookUpInvoice(NostrString invoice, std::function<void(LookUpInvoiceResponse)> onRes = nullptr, std::function<void(NostrString, NostrString)> onErr = nullptr);

    /**
     * List transactions
     * @param from The start time of the transactions to list (optional)
     * @param until The end time of the transactions to list (optional)
     * @param limit The maximum number of transactions to list (optional)
     * @param offset The offset of the transactions to list (optional)
     * @param unpaid Whether to list only unpaid transactions (optional)
     * @param type The type of transactions to list (optional)
     * @param onRes A callback that will be called when the transactions are listed (optional)
     * @param onErr A callback that will be called when the transactions listing fails (optional)
     */
    void listTransactions(unsigned long from = 0, unsigned long until = 0, int limit = 0, int offset = 0, bool unpaid = false, NostrString type = "",
                          std::function<void(ListTransactionsResponse)> onRes = nullptr, std::function<void(NostrString, NostrString)> onErr = nullptr);

    /**
     * Get the balance
     * @param onRes A callback that will be called when the balance is retrieved (optional)
     * @param onErr A callback that will be called when the balance retrieval fails (optional)
     */
    void getBalance(std::function<void(GetBalanceResponse)> onRes = nullptr, std::function<void(NostrString, NostrString)> onErr = nullptr);

    /**
     * Get the info
     * @param onRes A callback that will be called when the info is retrieved (optional)
     * @param onErr A callback that will be called when the info retrieval fails (optional)
     */
    void getInfo(std::function<void(GetInfoResponse)> onRes = nullptr, std::function<void(NostrString, NostrString)> onErr = nullptr);

    /**
     * Subscribe to payment notifications
     * @param onRes A callback that will be called when a payment notification is received
     * @param onErr A callback that will be called when an error is received (optional)
     */
    void subscribeNotifications(std::function<void(NotificationResponse)> onRes, std::function<void(NostrString, NostrString)> onErr);

  private:
    Transport *transport;
    NostrString sendEvent(SignedNostrEvent *ev);
    std::unique_ptr<NostrPool> pool;
    NWCData nwc;
    Nip47 nip47;
    NostrString accountPubKey;
    std::vector<std::unique_ptr<NWCResponseCallbackBase>> callbacks;
};
} 

#endif