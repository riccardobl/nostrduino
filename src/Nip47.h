#ifndef _NIP47_H
#define _NIP47_H
#include <Arduino.h>
#include "ArduinoJson.h"
#include "Nip04.h"
#include "NostrEvent.h"
#include "NostrString.h"
#include "NostrUtils.h"
#include <initializer_list>
#include <vector>
namespace nostr {

typedef struct s_Invoice {
    NostrString invoice;
    unsigned long amount;
} Invoice;

typedef struct s_TLVRecords {
    NostrString type;
    NostrString value;
} TLVRecords;

typedef struct s_KeySend {
    NostrString pubkey;
    unsigned long amount;
    NostrString preimage;
    std::initializer_list<TLVRecords> tlv;
} KeySend;

template <typename T> class Nip47Response {
  public:
    Nip47Response() {}

    Nip47Response(NostrString errorCode, NostrString errorMessage, NostrString resultType, T result) : errorCode(errorCode), errorMessage(errorMessage), resultType(resultType), result(result) {}
    NostrString errorCode = "";
    NostrString errorMessage = "";
    NostrString resultType = "";
    T result;
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
    unsigned long long amount;
    unsigned long long feesPaid;
    unsigned long long createdAt;
    unsigned long long expiresAt;
    JsonObject metadata;
} MakeInvoiceResponse;

typedef struct s_LookUpInvoiceResponse {
    NostrString type;
    NostrString invoice;
    NostrString description;
    NostrString descriptionHash;
    NostrString preimage;
    NostrString paymentHash;
    unsigned long long amount;
    unsigned long long feesPaid;
    unsigned long long createdAt;
    unsigned long long expiresAt;
    unsigned long long settledAt;

    JsonObject metadata;
} LookUpInvoiceResponse;

typedef struct s_Transaction {
    NostrString type;
    NostrString invoice;
    NostrString description;
    NostrString descriptionHash;
    NostrString preimage;
    NostrString paymentHash;
    unsigned long long amount;
    unsigned long long feesPaid;
    unsigned long long createdAt;
    unsigned long long expiresAt;
    unsigned long long settledAt;

    JsonObject metadata;

} Transaction;

typedef struct s_ListTransactionsResponse {
    std::vector<Transaction> transactions;
} ListTransactionsResponse;

typedef struct s_GetBalanceResponse {
    unsigned long long balance;
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
    NostrString pubkey;
    NostrString secret;
} NWCData;

class Nip47 {
  public:
    Nip47(){};
    Nip47(Nip04 nip04, NostrString servicePubKey, NostrString userPrivKey) : servicePubKey(servicePubKey), userPrivKey(userPrivKey), nip04(nip04) {}
    SignedNostrEvent payInvoice(NostrString invoice, unsigned long amount = -1);
    SignedNostrEvent multiPayInvoice(std::initializer_list<Invoice> invoices);
    SignedNostrEvent payKeySend(NostrString pubkey, unsigned long amount, NostrString preimage = "", std::initializer_list<TLVRecords> tlv = {});
    SignedNostrEvent multiPayKeySend(std::initializer_list<KeySend> keySends);
    SignedNostrEvent makeInvoice(unsigned long amount, NostrString description = "", NostrString descriptionHash = "", unsigned long expiry = 0);
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
    static void parseNWC(NostrString, NWCData &);

  private:
    NostrString servicePubKey;
    NostrString userPrivKey;
    Nip04 nip04;
    SignedNostrEvent createEvent(NostrString method, JsonDocument doc);
};
} 

#endif 