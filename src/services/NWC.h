#ifndef _NOSTR_NWC_H
#define _NOSTR_NWC_H
#include <functional>
#include <memory>

#include "ArduinoJson.h"
#include "Nip47.h"
#include "NostrEvent.h"
#include "NostrPool.h"
#include "NostrString.h"
#include "Utils.h"
#include "Transport.h"

namespace nostr {
    class NWCResponseCallbackBase {
       public:
        virtual ~NWCResponseCallbackBase() = default;
        virtual void call(Nip47 *nip47, SignedNostrEvent *ev){}
        std::function<void(NostrString,NostrString)> onErr = nullptr;
        unsigned long long timestampSeconds;
        NostrString eventId;
        NostrString subId;
        unsigned int n = 1;
    };

    template <typename T>
    class NWCResponseCallback : public NWCResponseCallbackBase {
       public:
        void call(Nip47 *nip47,SignedNostrEvent *ev)  override {
            Nip47Response<T> resp;
            nip47->parseResponse(ev,resp);
            if (NostrString_length(resp.errorCode)>0) {
                if(this->onErr!=nullptr){
                    this->onErr(resp.errorCode,resp.errorMessage);
                }
            } else {
                if(this->onRes!=nullptr){
                    this->onRes(resp.result);
                }
            }
        }
        std::function<void(T)> onRes = nullptr;
       
    };

    class NWC {
       public:
       ~NWC();
       NWC(Transport* transport,NostrString nwcUrl);
       void loop();
       void close();
       void payInvoice(NostrString invoice,
                       unsigned long amount = static_cast<unsigned long>(-1),
                       std::function<void(PayInvoiceResponse)> onRes = nullptr,
                       std::function<void(NostrString,NostrString)> onErr = nullptr);
       void multiPayInvoice(
           std::initializer_list<Invoice> invoices,
           std::function<void(MultiPayInvoiceResponse)> onRes = nullptr,
           std::function<void(NostrString,NostrString)> onErr = nullptr);
       void payKeySend(NostrString pubkey, unsigned long amount,
                       NostrString preimage = "",
                       std::initializer_list<TLVRecords> tlv = {},
                       std::function<void(PayKeySendResponse)> onRes = nullptr,
                       std::function<void(NostrString,NostrString)> onErr = nullptr);

       void multiPayKeySend(
           std::initializer_list<KeySend> keySends,
           std::function<void(MultiPayKeySendResponse)> onRes = nullptr,
           std::function<void(NostrString,NostrString)> onErr = nullptr);

       void makeInvoice(
           unsigned long amount, NostrString description = "",
           NostrString descriptionHash = "", unsigned long expiry = 0,
           std::function<void(MakeInvoiceResponse)> onRes = nullptr,
           std::function<void(NostrString,NostrString)> onErr = nullptr);

       void lookUpPaymentHash(
           NostrString paymentHash,
           std::function<void(LookUpInvoiceResponse)> onRes = nullptr,
           std::function<void(NostrString,NostrString)> onErr = nullptr);

       void lookUpInvoice(
           NostrString invoice,
           std::function<void(LookUpInvoiceResponse)> onRes = nullptr,
           std::function<void(NostrString,NostrString)> onErr = nullptr);

       void listTransactions(
           unsigned long from = 0, unsigned long until = 0, int limit = 0,
           int offset = 0, bool unpaid = false, NostrString type = "",
           std::function<void(ListTransactionsResponse)> onRes = nullptr,
           std::function<void(NostrString,NostrString)> onErr = nullptr);

       void getBalance(std::function<void(GetBalanceResponse)> onRes = nullptr,
                       std::function<void(NostrString,NostrString)> onErr = nullptr);

       void getInfo(std::function<void(GetInfoResponse)> onRes = nullptr,
                    std::function<void(NostrString,NostrString)> onErr = nullptr);

      private:
      Transport* transport;
       NostrString sendEvent(SignedNostrEvent *ev);
       std::unique_ptr<NostrPool> pool;
       NWCData nwc;
       Nip47 nip47;
       NostrString accountPubKey;
        std::vector<std::unique_ptr<NWCResponseCallbackBase>> callbacks;
    };
}  // namespace nostr

#endif