#ifndef _NIP47_H
#define _NIP47_H
#include <Arduino.h>
#include "ArduinoJson.h"
#include "Nip04.h"
#include "Nip44.h"
#include "NostrEvent.h"
#include "NostrAmount.h"
#include "NostrString.h"
#include "NostrUtils.h"
#include <initializer_list>
#include <vector>
namespace nostr {

enum class Nip47Encryption {
    Nip04,
    Nip44V2
};

typedef struct s_Invoice {
    NostrString invoice;
    MilliSats amount;
    NostrString id;

    s_Invoice() : amount(MILLISATS_UNSPECIFIED) {}
    s_Invoice(NostrString invoice, MilliSats amount = MILLISATS_UNSPECIFIED,
              NostrString id = "")
        : invoice(invoice), amount(amount), id(id) {}
} Invoice;

typedef struct s_TLVRecords {
    uint64_t type;
    NostrString value;
} TLVRecords;

typedef struct s_KeySend {
    NostrString pubkey;
    MilliSats amount;
    NostrString preimage;
    std::vector<TLVRecords> tlv;
    NostrString id;
} KeySend;

template <typename T> class Nip47Response {
  public:
    Nip47Response() : result{} {}

    Nip47Response(NostrString errorCode, NostrString errorMessage, NostrString resultType, T result) : errorCode(errorCode), errorMessage(errorMessage), resultType(resultType), result(result) {}
    NostrString errorCode = "";
    NostrString errorMessage = "";
    NostrString resultType = "";
    T result{};
};

typedef struct s_PayInvoiceResponse {
    NostrString preimage;
} PayInvoiceResponse;

typedef struct s_MultiPayInvoiceResponse {
    NostrString preimage;
    NostrString d;
} MultiPayInvoiceResponse;

typedef struct s_PayKeySendResponse {
    NostrString preimage;
} PayKeySendResponse;

typedef struct s_MultiPayKeySendResponse {
    NostrString preimage;
    NostrString d;

} MultiPayKeySendResponse;

typedef struct s_MakeInvoiceResponse {
    NostrString type;
    NostrString invoice;
    NostrString description;
    NostrString descriptionHash;
    NostrString preimage;
    NostrString paymentHash;
    MilliSats amount;
    MilliSats feesPaid;
    unsigned long long createdAt;
    unsigned long long expiresAt;
    JsonDocument metadata;
} MakeInvoiceResponse;

typedef struct s_LookUpInvoiceResponse {
    NostrString type;
    NostrString invoice;
    NostrString description;
    NostrString descriptionHash;
    NostrString preimage;
    NostrString paymentHash;
    MilliSats amount;
    MilliSats feesPaid;
    unsigned long long createdAt;
    unsigned long long expiresAt;
    unsigned long long settledAt;

    JsonDocument metadata;
} LookUpInvoiceResponse;

typedef struct s_Transaction {
    NostrString type;
    NostrString invoice;
    NostrString description;
    NostrString descriptionHash;
    NostrString preimage;
    NostrString paymentHash;
    MilliSats amount;
    MilliSats feesPaid;
    unsigned long long createdAt;
    unsigned long long expiresAt;
    unsigned long long settledAt;

    JsonDocument metadata;

} Transaction;

typedef struct s_ListTransactionsResponse {
    std::vector<Transaction> transactions;
} ListTransactionsResponse;

typedef struct s_GetBalanceResponse {
    MilliSats balance;
} GetBalanceResponse;

typedef struct s_GetInfoResponse {
    NostrString alias;
    NostrString color;
    NostrString pubkey;
    NostrString network;
    unsigned long long blockHeight;
    NostrString blockHash;
    std::vector<NostrString> methods;
    std::vector<NostrString> notifications;
} GetInfoResponse;

typedef struct s_NWCData {
    NostrString relay;
    std::vector<NostrString> relays;
    NostrString pubkey;
    NostrString secret;
} NWCData;


typedef struct s_Nip47Notification {
    NostrString notificationType;
    NostrString type;
    NostrString invoice;
    NostrString description;
    NostrString descriptionHash;
    NostrString preimage;
    NostrString paymentHash;
    MilliSats amount;
    MilliSats feesPaid;
    unsigned long long createdAt;
    unsigned long long settledAt;
} Nip47Notification;

typedef struct s_NotificationResponse {
    Nip47Notification notification;
} NotificationResponse;


class Nip47 {
  public:
    Nip47(){};
    Nip47(Nip04 nip04, NostrString servicePubKey, NostrString userPrivKey) : servicePubKey(servicePubKey), userPrivKey(userPrivKey), nip04(nip04) {}
    void setEncryption(Nip47Encryption encryption, bool includeEncryptionTag = true);
    Nip47Encryption getEncryption() const { return encryption; }
    SignedNostrEvent payInvoice(NostrString invoice, MilliSats amount = MILLISATS_UNSPECIFIED);
    SignedNostrEvent multiPayInvoice(std::initializer_list<Invoice> invoices);
    SignedNostrEvent multiPayInvoice(const std::vector<Invoice> &invoices);
    SignedNostrEvent payKeySend(NostrString pubkey, MilliSats amount, NostrString preimage = "", std::initializer_list<TLVRecords> tlv = {});
    SignedNostrEvent payKeySend(NostrString pubkey, MilliSats amount, NostrString preimage,
                                const std::vector<TLVRecords> &tlv);
    SignedNostrEvent multiPayKeySend(std::initializer_list<KeySend> keySends);
    SignedNostrEvent multiPayKeySend(const std::vector<KeySend> &keySends);
    SignedNostrEvent makeInvoice(MilliSats amount, NostrString description = "", NostrString descriptionHash = "", unsigned long expiry = 0);
    SignedNostrEvent lookUpPaymentHash(NostrString paymentHash);
    SignedNostrEvent lookUpInvoice(NostrString invoice);
    SignedNostrEvent listTransactions(unsigned long from = 0, unsigned long until = 0, int limit = 0, int offset = 0, bool unpaid = false, NostrString type = "");
    SignedNostrEvent getBalance();
    SignedNostrEvent getInfo();

    void parseResponse(SignedNostrEvent *response, Nip47Response<PayInvoiceResponse> &out);
    void parseResponse(SignedNostrEvent *response, Nip47Response<MultiPayInvoiceResponse> &out);
    void parseResponse(SignedNostrEvent *response, Nip47Response<PayKeySendResponse> &out);
    void parseResponse(SignedNostrEvent *response, Nip47Response<MultiPayKeySendResponse> &out);
    void parseResponse(SignedNostrEvent *response, Nip47Response<MakeInvoiceResponse> &out);
    void parseResponse(SignedNostrEvent *response, Nip47Response<LookUpInvoiceResponse> &out);
    void parseResponse(SignedNostrEvent *response, Nip47Response<ListTransactionsResponse> &out);
    void parseResponse(SignedNostrEvent *response, Nip47Response<GetBalanceResponse> &out);
    void parseResponse(SignedNostrEvent *response, Nip47Response<GetInfoResponse> &out);
    void parseResponse(SignedNostrEvent *response, Nip47Response<NotificationResponse> &out);
    static bool parseNWC(NostrString, NWCData &);

  private:
    NostrString servicePubKey;
    NostrString userPrivKey;
    Nip04 nip04;
    Nip44 nip44;
    Nip47Encryption encryption = Nip47Encryption::Nip04;
    bool includeEncryptionTag = false;
    SignedNostrEvent createEvent(NostrString method, JsonDocument doc);
    NostrString encryptContent(NostrString content);
    NostrString decryptContent(NostrString content);
    bool parseResponseEnvelope(SignedNostrEvent *response, const char *expectedResultType,
                               JsonDocument &document, JsonObjectConst &result,
                               NostrString &errorCode, NostrString &errorMessage);
    std::function<void(Nip47Notification)> notificationCallback;
};
} 

#endif 
