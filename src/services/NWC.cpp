#include "NWC.h"
#include "Nip47.h"

using namespace nostr;

namespace {

bool hasSingleTagValue(const SignedNostrEvent *event, const char *key, const NostrString &expectedValue) {
    size_t matchingTags = 0;
    for (const NostrEventTag &tag : event->getTags()->getTags()) {
        if (!NostrString_equals(tag.key, key)) continue;
        ++matchingTags;
        if (tag.value.size() != 1 || !NostrString_equals(tag.value[0], expectedValue)) return false;
    }
    return matchingTags == 1;
}

} // namespace

NWC::~NWC() {
    this->close();
}

NWC::NWC(Transport *transport, NostrString nwcUrl) {
    this->pool = std::unique_ptr<NostrPool>(new NostrPool(transport));
    Nip47::parseNWC(nwcUrl, this->nwc);

    Utils::log("NWC RELAY: " + this->nwc.relay);
    Utils::log("NWC PUBKEY: " + this->nwc.pubkey);
    Utils::log("NWC PRIVKEY: ******");

    NostrString accountSecret = this->nwc.secret;

    this->nip47 = Nip47(Nip04(), this->nwc.pubkey, accountSecret);
    this->accountPubKey = Utils::getPublicKey(accountSecret);
    this->discoverEncryption();
}

void NWC::discoverEncryption() {
    this->infoSubId = this->pool->subscribeMany(
        {this->nwc.relay},
        {{{"kinds", {NWC_INFO_KIND}}, {"authors", {this->nwc.pubkey}}, {"limit", {"1"}}}},
        [this](const NostrString &, SignedNostrEvent *event) {
            if (event == nullptr || event->getKind() != 13194 ||
                !NostrString_equals(event->getPubkey(), this->nwc.pubkey) || !event->verify()) return;

            bool hasEncryptionTag = false;
            bool supportsNip44 = false;
            bool supportsNip04 = false;
            for (const NostrEventTag &tag : event->getTags()->getTags()) {
                if (!NostrString_equals(tag.key, "encryption")) continue;
                hasEncryptionTag = true;
                for (const NostrString &advertised : tag.value) {
                    std::vector<NostrString> modes;
                    NostrString_split(advertised, ' ', modes);
                    for (const NostrString &mode : modes) {
                        if (NostrString_equals(mode, "nip44_v2")) supportsNip44 = true;
                        if (NostrString_equals(mode, "nip04")) supportsNip04 = true;
                    }
                }
            }

            if (supportsNip44) {
                this->selectEncryption(Nip47Encryption::Nip44V2, true);
            } else if (supportsNip04) {
                this->selectEncryption(Nip47Encryption::Nip04, true);
            } else if (!hasEncryptionTag) {
                this->selectEncryption(Nip47Encryption::Nip04, false);
            } else {
                this->rejectUnsupportedEncryption();
            }
        },
        [this](const NostrString &, const NostrString &reason) {
            NostrString message = "Unable to read wallet info event";
            if (NostrString_length(reason) > 0) message += ": " + reason;
            this->failEncryption("OTHER", message);
        },
        [this](const NostrString &) {
            this->failEncryption("OTHER", "Wallet info event was not found");
        });
}

void NWC::finishInfoSubscription() {
    if (NostrString_length(this->infoSubId) > 0) {
        NostrString subscription = this->infoSubId;
        this->infoSubId = "";
        this->pool->closeSubscription(subscription);
    }
}

void NWC::selectEncryption(Nip47Encryption selectedEncryption, bool includeEncryptionTag) {
    if (this->encryptionReady || this->encryptionFailed) return;
    this->nip47.setEncryption(selectedEncryption, includeEncryptionTag);
    this->encryptionReady = true;
    this->finishInfoSubscription();

    std::vector<PendingRequest> pending;
    pending.swap(this->pendingRequests);
    for (PendingRequest &request : pending) {
        if (request.send) request.send();
    }
}

void NWC::rejectUnsupportedEncryption() {
    this->failEncryption("UNSUPPORTED_ENCRYPTION", "Wallet advertised no supported encryption scheme");
}

void NWC::failEncryption(NostrString errorCode, NostrString errorMessage) {
    if (this->encryptionReady || this->encryptionFailed) return;
    this->encryptionFailed = true;
    this->encryptionErrorCode = errorCode;
    this->encryptionErrorMessage = errorMessage;
    this->finishInfoSubscription();

    std::vector<PendingRequest> pending;
    pending.swap(this->pendingRequests);
    for (PendingRequest &request : pending) {
        if (request.onErr) request.onErr(this->encryptionErrorCode, this->encryptionErrorMessage);
    }
}

bool NWC::deferUntilEncryptionReady(std::function<void()> send,
    std::function<void(NostrString, NostrString)> onErr) {
    if (this->encryptionReady) return false;
    if (this->encryptionFailed) {
        if (onErr) onErr(this->encryptionErrorCode, this->encryptionErrorMessage);
        return true;
    }
    this->pendingRequests.push_back({send, onErr});
    return true;
}

void NWC::close() {
    this->pool->close();
    this->pendingRequests.clear();
    this->callbacks.clear();
}

void NWC::loop() {
    this->pool->loop();
    for (auto it = this->callbacks.begin(); it != this->callbacks.end();) {
        if (it->get()->n == 0) {
            NostrString subId = it->get()->subId;
            this->pool->closeSubscription(subId);
            it = this->callbacks.erase(it);
            continue;
        }
        if (Utils::unixTimeSeconds() - it->get()->timestampSeconds > it->get()->timeoutSeconds) {
            NostrString subId = it->get()->subId;
            this->pool->closeSubscription(subId);
            if (it->get()->onErr != nullptr) {
                it->get()->onErr("OTHER", "timeout");
            }
            it = this->callbacks.erase(it);
        } else {
            it++;
        }
    }
}

NostrString NWC::sendEvent(SignedNostrEvent *event) {
    NostrString subId = this->pool->subscribeMany(
        {this->nwc.relay}, {{{"kinds", {NWC_RESPONSE_KIND}}, {"authors", {this->nwc.pubkey}}, {"#p", {this->accountPubKey}}, {"#e", {event->getId()}}}},
        [&](const String &subId, nostr::SignedNostrEvent *event) {
            if (event->getKind() != 23195) return;
            if (!NostrString_equals(event->getPubkey(), this->nwc.pubkey)) return;
            if (!event->verify()) return;
            if (!hasSingleTagValue(event, "p", this->accountPubKey)) return;

            for (auto it = this->callbacks.begin(); it != this->callbacks.end(); it++) {
                if (hasSingleTagValue(event, "e", it->get()->eventId)) {
                    if (it->get()->n == 0) break;
                    it->get()->call(&this->nip47, event);
                    --it->get()->n;
                    break;
                }
            }
        },
        [&](const String &subId, const String &reason) { Utils::log("NWC: closed subscription: " + reason); }, [&](const String &subId) { Utils::log("NWC: EOS"); });
    this->pool->publish({this->nwc.relay}, event, [&](const NostrString &eventId, bool status, const NostrString &msg) {
        if (!status) {
            Utils::log("NWC: error sending event: " + msg);
        } else {
            Utils::log("NWC: event sent: " + eventId);
        }
    });
    return subId;
}

void NWC::subscribeNotifications(std::function<void(NotificationResponse)> onRes, std::function<void(NostrString, NostrString)> onErr) {
    if (this->deferUntilEncryptionReady(
            [this, onRes, onErr]() { this->subscribeNotifications(onRes, onErr); }, onErr)) return;
    std::unique_ptr<NWCResponseCallback<NotificationResponse>> callback(new NWCResponseCallback<NotificationResponse>());
    callback->onRes = onRes;
    callback->onErr = onErr;
    callback->timestampSeconds = Utils::unixTimeSeconds();
    callback->timeoutSeconds = NWC_INFINITE_TIMEOUT; // notification subscriptions don't timeout, they can come after any amount of time
    callback->subId = pool->subscribeMany(
        {this->nwc.relay}, {{{"kinds", {NWC_NOTIFICATION_KIND}}, {"authors", {this->nwc.pubkey}}, {"#p", {this->accountPubKey}}}},
        [&](const NostrString &subId, SignedNostrEvent *event) {
            if (event->getKind() != 23196) return;
            if (!NostrString_equals(event->getPubkey(), this->nwc.pubkey))  return;
            if (!event->verify()) return;
            if (!hasSingleTagValue(event, "p", this->accountPubKey)) return;
            NostrString subRef = event->getSubId();
            for (auto it = this->callbacks.begin(); it != this->callbacks.end(); it++) {
                if (NostrString_equals(it->get()->subId, subRef)) {
                    it->get()->call(&this->nip47, event);
                    break;
                }
            }
        },
        [&](const String &subId, const String &reason) { Utils::log("NWC: closed notification subscription: " + reason); }, [&](const String &subId) { Utils::log("NWC: notification EOS"); });
    this->callbacks.push_back(std::move(callback));
}

void NWC::payInvoice(NostrString invoice, MilliSats amount, std::function<void(PayInvoiceResponse)> onRes, std::function<void(NostrString, NostrString)> onErr) {
    if (!isOptionalMilliSatsValid(amount)) {
        if (onErr) onErr("INVALID_AMOUNT", "Invalid millisatoshi amount");
        return;
    }
    if (this->deferUntilEncryptionReady(
            [this, invoice, amount, onRes, onErr]() { this->payInvoice(invoice, amount, onRes, onErr); }, onErr)) return;
    SignedNostrEvent ev = this->nip47.payInvoice(invoice, amount);
    std::unique_ptr<NWCResponseCallback<PayInvoiceResponse>> callback(new NWCResponseCallback<PayInvoiceResponse>());
    callback->onRes = onRes;
    callback->onErr = onErr;
    callback->timestampSeconds = Utils::unixTimeSeconds();
    callback->eventId = ev.getId();
    callback->n = 1;
    callback->subId = this->sendEvent(&ev);
    this->callbacks.push_back(std::move(callback));
}

void NWC::multiPayInvoice(std::initializer_list<Invoice> invoices, std::function<void(MultiPayInvoiceResponse)> onRes, std::function<void(NostrString, NostrString)> onErr) {
    this->multiPayInvoice(std::vector<Invoice>(invoices), onRes, onErr);
}

void NWC::multiPayInvoice(const std::vector<Invoice> &invoices, std::function<void(MultiPayInvoiceResponse)> onRes,
                          std::function<void(NostrString, NostrString)> onErr) {
    for (const Invoice &invoice : invoices) {
        if (!isRequiredMilliSatsValid(invoice.amount)) {
            if (onErr) onErr("INVALID_AMOUNT", "Invalid millisatoshi amount");
            return;
        }
    }
    if (this->deferUntilEncryptionReady(
            [this, invoices, onRes, onErr]() { this->multiPayInvoice(invoices, onRes, onErr); }, onErr)) return;
    SignedNostrEvent ev = this->nip47.multiPayInvoice(invoices);
    std::unique_ptr<NWCResponseCallback<MultiPayInvoiceResponse>> callback(new NWCResponseCallback<MultiPayInvoiceResponse>());
    callback->onRes = onRes;
    callback->onErr = onErr;
    callback->timestampSeconds = Utils::unixTimeSeconds();
    callback->eventId = ev.getId();
    callback->n = invoices.size();
    callback->subId = this->sendEvent(&ev);
    this->callbacks.push_back(std::move(callback));
}

void NWC::payKeySend(NostrString pubkey, MilliSats amount, NostrString preimage, std::initializer_list<TLVRecords> tlv, std::function<void(PayKeySendResponse)> onRes,
                     std::function<void(NostrString, NostrString)> onErr) {
    this->payKeySend(pubkey, amount, preimage, std::vector<TLVRecords>(tlv), onRes, onErr);
}

void NWC::payKeySend(NostrString pubkey, MilliSats amount, NostrString preimage,
                     const std::vector<TLVRecords> &tlv, std::function<void(PayKeySendResponse)> onRes,
                     std::function<void(NostrString, NostrString)> onErr) {
    if (!isRequiredMilliSatsValid(amount)) {
        if (onErr) onErr("INVALID_AMOUNT", "Invalid millisatoshi amount");
        return;
    }
    if (this->deferUntilEncryptionReady(
            [this, pubkey, amount, preimage, tlv, onRes, onErr]() {
                this->payKeySend(pubkey, amount, preimage, tlv, onRes, onErr);
            }, onErr)) return;
    SignedNostrEvent ev = this->nip47.payKeySend(pubkey, amount, preimage, tlv);
    std::unique_ptr<NWCResponseCallback<PayKeySendResponse>> callback(new NWCResponseCallback<PayKeySendResponse>());
    callback->onRes = onRes;
    callback->onErr = onErr;
    callback->timestampSeconds = Utils::unixTimeSeconds();
    callback->eventId = ev.getId();
    callback->n = 1;
    callback->subId = this->sendEvent(&ev);
    this->callbacks.push_back(std::move(callback));
}

void NWC::multiPayKeySend(std::initializer_list<KeySend> keySends, std::function<void(MultiPayKeySendResponse)> onRes, std::function<void(NostrString, NostrString)> onErr) {
    this->multiPayKeySend(std::vector<KeySend>(keySends), onRes, onErr);
}

void NWC::multiPayKeySend(const std::vector<KeySend> &keySends, std::function<void(MultiPayKeySendResponse)> onRes,
                          std::function<void(NostrString, NostrString)> onErr) {
    for (const KeySend &keySend : keySends) {
        if (!isRequiredMilliSatsValid(keySend.amount)) {
            if (onErr) onErr("INVALID_AMOUNT", "Invalid millisatoshi amount");
            return;
        }
    }
    if (this->deferUntilEncryptionReady(
            [this, keySends, onRes, onErr]() { this->multiPayKeySend(keySends, onRes, onErr); }, onErr)) return;
    SignedNostrEvent ev = this->nip47.multiPayKeySend(keySends);
    std::unique_ptr<NWCResponseCallback<MultiPayKeySendResponse>> callback(new NWCResponseCallback<MultiPayKeySendResponse>());
    callback->onRes = onRes;
    callback->onErr = onErr;
    callback->timestampSeconds = Utils::unixTimeSeconds();
    callback->eventId = ev.getId();
    callback->n = keySends.size();
    callback->subId = this->sendEvent(&ev);
    this->callbacks.push_back(std::move(callback));
}

void NWC::makeInvoice(MilliSats amount, NostrString description, NostrString descriptionHash, unsigned long expiry, std::function<void(MakeInvoiceResponse)> onRes,
                      std::function<void(NostrString, NostrString)> onErr) {
    if (!isRequiredMilliSatsValid(amount)) {
        if (onErr) onErr("INVALID_AMOUNT", "Invalid millisatoshi amount");
        return;
    }
    if (this->deferUntilEncryptionReady(
            [this, amount, description, descriptionHash, expiry, onRes, onErr]() {
                this->makeInvoice(amount, description, descriptionHash, expiry, onRes, onErr);
            }, onErr)) return;
    SignedNostrEvent ev = this->nip47.makeInvoice(amount, description, descriptionHash, expiry);
    std::unique_ptr<NWCResponseCallback<MakeInvoiceResponse>> callback(new NWCResponseCallback<MakeInvoiceResponse>());
    callback->onRes = onRes;
    callback->onErr = onErr;
    callback->timestampSeconds = Utils::unixTimeSeconds();
    callback->eventId = ev.getId();
    callback->n = 1;
    callback->subId = this->sendEvent(&ev);
    this->callbacks.push_back(std::move(callback));
}

void NWC::lookUpPaymentHash(NostrString paymentHash, std::function<void(LookUpInvoiceResponse)> onRes, std::function<void(NostrString, NostrString)> onErr) {
    if (this->deferUntilEncryptionReady(
            [this, paymentHash, onRes, onErr]() { this->lookUpPaymentHash(paymentHash, onRes, onErr); }, onErr)) return;
    SignedNostrEvent ev = this->nip47.lookUpPaymentHash(paymentHash);
    std::unique_ptr<NWCResponseCallback<LookUpInvoiceResponse>> callback(new NWCResponseCallback<LookUpInvoiceResponse>());
    callback->onRes = onRes;
    callback->onErr = onErr;
    callback->timestampSeconds = Utils::unixTimeSeconds();
    callback->eventId = ev.getId();
    callback->n = 1;
    callback->subId = this->sendEvent(&ev);
    this->callbacks.push_back(std::move(callback));
}

void NWC::lookUpInvoice(NostrString invoice, std::function<void(LookUpInvoiceResponse)> onRes, std::function<void(NostrString, NostrString)> onErr) {
    if (this->deferUntilEncryptionReady(
            [this, invoice, onRes, onErr]() { this->lookUpInvoice(invoice, onRes, onErr); }, onErr)) return;
    SignedNostrEvent ev = this->nip47.lookUpInvoice(invoice);
    std::unique_ptr<NWCResponseCallback<LookUpInvoiceResponse>> callback(new NWCResponseCallback<LookUpInvoiceResponse>());
    callback->onRes = onRes;
    callback->onErr = onErr;
    callback->timestampSeconds = Utils::unixTimeSeconds();
    callback->eventId = ev.getId();
    callback->n = 1;
    callback->subId = this->sendEvent(&ev);
    this->callbacks.push_back(std::move(callback));
}

void NWC::listTransactions(unsigned long from, unsigned long until, int limit, int offset, bool unpaid, NostrString type, std::function<void(ListTransactionsResponse)> onRes,
                           std::function<void(NostrString, NostrString)> onErr) {
    if (this->deferUntilEncryptionReady(
            [this, from, until, limit, offset, unpaid, type, onRes, onErr]() {
                this->listTransactions(from, until, limit, offset, unpaid, type, onRes, onErr);
            }, onErr)) return;
    SignedNostrEvent ev = this->nip47.listTransactions(from, until, limit, offset, unpaid, type);
    std::unique_ptr<NWCResponseCallback<ListTransactionsResponse>> callback(new NWCResponseCallback<ListTransactionsResponse>());
    callback->onRes = onRes;
    callback->onErr = onErr;
    callback->timestampSeconds = Utils::unixTimeSeconds();
    callback->eventId = ev.getId();
    callback->n = 1;
    callback->subId = this->sendEvent(&ev);
    this->callbacks.push_back(std::move(callback));
}

void NWC::getBalance(std::function<void(GetBalanceResponse)> onRes, std::function<void(NostrString, NostrString)> onErr) {
    if (this->deferUntilEncryptionReady(
            [this, onRes, onErr]() { this->getBalance(onRes, onErr); }, onErr)) return;
    SignedNostrEvent ev = this->nip47.getBalance();
    std::unique_ptr<NWCResponseCallback<GetBalanceResponse>> callback(new NWCResponseCallback<GetBalanceResponse>());
    callback->onRes = onRes;
    callback->onErr = onErr;
    callback->timestampSeconds = Utils::unixTimeSeconds();
    callback->eventId = ev.getId();
    callback->n = 1;
    callback->subId = this->sendEvent(&ev);
    this->callbacks.push_back(std::move(callback));
}

void NWC::getInfo(std::function<void(GetInfoResponse)> onRes, std::function<void(NostrString, NostrString)> onErr) {
    if (this->deferUntilEncryptionReady(
            [this, onRes, onErr]() { this->getInfo(onRes, onErr); }, onErr)) return;
    SignedNostrEvent ev = this->nip47.getInfo();
    std::unique_ptr<NWCResponseCallback<GetInfoResponse>> callback(new NWCResponseCallback<GetInfoResponse>());
    callback->onRes = onRes;
    callback->onErr = onErr;
    callback->timestampSeconds = Utils::unixTimeSeconds();
    callback->eventId = ev.getId();
    callback->n = 1;
    callback->subId = this->sendEvent(&ev);
    this->callbacks.push_back(std::move(callback));
}
