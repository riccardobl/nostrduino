#include "NWC.h"
#include "Nip47.h"
#include <algorithm>

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

NWC::NWC(Transport *transport, NostrString nwcUrl, const NostrLimits &limits,
         NWCEncryptionPolicy encryptionPolicy)
    : transport(transport), limits(limits), encryptionPolicy(encryptionPolicy) {
    this->validConnection = Nip47::parseNWC(nwcUrl, this->nwc);
    if (!this->validConnection) return;
    this->pool = std::unique_ptr<NostrPool>(new NostrPool(transport, NWC_DEFAULT_TIMEOUT, limits));

    Utils::log("NWC RELAY: " + this->nwc.relay);
    Utils::log("NWC PUBKEY: " + this->nwc.pubkey);
    Utils::log("NWC PRIVKEY: ******");

    NostrString accountSecret = this->nwc.secret;

    this->nip47 = Nip47(Nip04(), this->nwc.pubkey, accountSecret);
    this->accountPubKey = Utils::getPublicKey(accountSecret);
    if (NostrString_length(this->accountPubKey) != 64) {
        this->validConnection = false;
        this->pool.reset();
        return;
    }
    this->discoveryStartedAt = Utils::unixTimeSeconds();
    this->discoverEncryption();
}

void NWC::discoverEncryption() {
    this->infoSubId = this->pool->subscribeMany(
        {this->nwc.relay},
        {{{"kinds", {NWC_INFO_KIND}}, {"authors", {this->nwc.pubkey}}}},
        [this](const NostrString &, SignedNostrEvent *event) {
            if (event == nullptr || event->getKind() != 13194 ||
                !NostrString_equals(event->getPubkey(), this->nwc.pubkey) || !event->verify()) return;

            this->sawInfoEvent = true;

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

            this->sawNip44 = this->sawNip44 || supportsNip44;
            this->sawNip04 = this->sawNip04 || supportsNip04;
            this->sawLegacyWithoutTag = this->sawLegacyWithoutTag || !hasEncryptionTag;
        },
        [this](const NostrString &, const NostrString &reason) {
            if (this->sawInfoEvent) this->finishEncryptionDiscovery();
            if (this->encryptionReady || this->encryptionFailed) return;
            NostrString message = "Unable to read wallet info event";
            if (NostrString_length(reason) > 0) message += ": " + reason;
            this->failEncryption("OTHER", message);
        },
        [this](const NostrString &) {
            this->finishEncryptionDiscovery();
        });
}

void NWC::finishEncryptionDiscovery() {
    if (this->encryptionReady || this->encryptionFailed) return;
    if (this->sawNip44) {
        this->selectEncryption(Nip47Encryption::Nip44V2, true);
        return;
    }
    if (this->encryptionPolicy == NWCEncryptionPolicy::AllowLegacyNip04) {
        if (this->sawNip04) {
            this->selectEncryption(Nip47Encryption::Nip04, true);
            return;
        }
        if (this->sawLegacyWithoutTag) {
            this->selectEncryption(Nip47Encryption::Nip04, false);
            return;
        }
    }
    if (this->sawInfoEvent) {
        this->rejectUnsupportedEncryption();
    } else {
        this->failEncryption("OTHER", "Wallet info event was not found");
    }
}

void NWC::finishInfoSubscription() {
    if (this->pool != nullptr && NostrString_length(this->infoSubId) > 0) {
        NostrString subscription = this->infoSubId;
        this->infoSubId = "";
        this->pool->closeSubscription(subscription);
    }
}

void NWC::selectEncryption(Nip47Encryption selectedEncryption, bool includeEncryptionTag) {
    if (this->encryptionReady || this->encryptionFailed) return;
    this->nip47.setEncryption(selectedEncryption, includeEncryptionTag);
    this->notificationKind = selectedEncryption == Nip47Encryption::Nip44V2 ? 23197 : 23196;
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
    if (!this->validConnection) {
        if (onErr) onErr("INVALID_CONNECTION", "Invalid NWC connection string");
        return true;
    }
    if (this->encryptionReady) return false;
    if (this->encryptionFailed) {
        if (onErr) onErr(this->encryptionErrorCode, this->encryptionErrorMessage);
        return true;
    }
    if (this->pendingRequests.size() >= this->limits.maxNwcPendingRequests) {
        if (onErr) onErr("QUEUE_FULL", "NWC pending request queue is full");
        return true;
    }
    this->pendingRequests.push_back({send, onErr});
    return true;
}

bool NWC::rejectIfCallbackQueueFull(std::function<void(NostrString, NostrString)> onErr) {
    if (this->callbacks.size() < this->limits.maxNwcPendingRequests) return false;
    if (onErr) onErr("QUEUE_FULL", "NWC callback queue is full");
    return true;
}

void NWC::close() {
    if (this->closed) return;
    if (this->poolDispatchDepth > 0) {
        this->deferredClose = true;
        return;
    }
    this->performClose();
}

void NWC::performClose() {
    if (this->closed) return;
    this->closed = true;
    this->deferredClose = false;
    if (this->pool != nullptr) this->pool->close();
    this->pendingRequests.clear();
    this->callbacks.clear();
}

void NWC::removeCallback(const std::shared_ptr<NWCResponseCallbackBase> &callback) {
    this->callbacks.erase(
        std::remove(this->callbacks.begin(), this->callbacks.end(), callback),
        this->callbacks.end());
}

void NWC::loop() {
    if (this->closed || !this->validConnection || this->pool == nullptr) return;
    ++this->poolDispatchDepth;
    this->pool->loop();
    --this->poolDispatchDepth;
    if (this->deferredClose) {
        this->performClose();
        return;
    }
    if (!this->encryptionReady && !this->encryptionFailed &&
        Utils::unixTimeSeconds() - this->discoveryStartedAt > NWC_DEFAULT_TIMEOUT) {
        this->finishEncryptionDiscovery();
        if (!this->encryptionReady && !this->encryptionFailed) {
            this->failEncryption("OTHER", "timeout");
        }
        return;
    }

    const std::vector<std::shared_ptr<NWCResponseCallbackBase>> snapshot = this->callbacks;
    for (const std::shared_ptr<NWCResponseCallbackBase> &callback : snapshot) {
        if (this->closed) break;
        if (std::find(this->callbacks.begin(), this->callbacks.end(), callback) == this->callbacks.end()) continue;
        if (callback->n == 0) {
            NostrString subId = callback->subId;
            this->removeCallback(callback);
            this->pool->closeSubscription(subId);
            continue;
        }
        if (Utils::unixTimeSeconds() - callback->timestampSeconds > callback->timeoutSeconds) {
            NostrString subId = callback->subId;
            std::function<void(NostrString, NostrString)> onErr = callback->onErr;
            this->removeCallback(callback);
            this->pool->closeSubscription(subId);
            if (onErr != nullptr) onErr("OTHER", "timeout");
        }
    }
}

NostrString NWC::sendEvent(SignedNostrEvent *event,
                           std::function<void(NostrString, NostrString)> onErr) {
    NostrString subId = this->pool->subscribeMany(
        {this->nwc.relay}, {{{"kinds", {NWC_RESPONSE_KIND}}, {"authors", {this->nwc.pubkey}}, {"#p", {this->accountPubKey}}, {"#e", {event->getId()}}}},
        [&](const String &subId, nostr::SignedNostrEvent *event) {
            if (event->getKind() != 23195) return;
            if (!NostrString_equals(event->getPubkey(), this->nwc.pubkey)) return;
            if (!event->verify()) return;
            if (!hasSingleTagValue(event, "p", this->accountPubKey)) return;

            const std::vector<std::shared_ptr<NWCResponseCallbackBase>> snapshot = this->callbacks;
            for (const std::shared_ptr<NWCResponseCallbackBase> &callback : snapshot) {
                if (hasSingleTagValue(event, "e", callback->eventId)) {
                    if (callback->n == 0) break;
                    --callback->n;
                    if (callback->n == 0) {
                        NostrString responseSubId = callback->subId;
                        this->removeCallback(callback);
                        this->pool->closeSubscription(responseSubId);
                    }
                    callback->call(&this->nip47, event);
                    break;
                }
            }
        },
        [&](const String &subId, const String &reason) { Utils::log("NWC: closed subscription: " + reason); }, [&](const String &subId) { Utils::log("NWC: EOS"); });
    if (NostrString_length(subId) == 0) {
        if (onErr) onErr("QUEUE_FULL", "Relay queue is full");
        return "";
    }
    if (!this->pool->publish({this->nwc.relay}, event, [&](const NostrString &eventId, bool status, const NostrString &msg) {
        if (!status) {
            Utils::log("NWC: error sending event: " + msg);
        } else {
            Utils::log("NWC: event sent: " + eventId);
        }
    })) {
        this->pool->closeSubscription(subId);
        if (onErr) onErr("QUEUE_FULL", "Relay queue is full");
        return "";
    }
    return subId;
}

void NWC::subscribeNotifications(std::function<void(NotificationResponse)> onRes, std::function<void(NostrString, NostrString)> onErr) {
    if (this->deferUntilEncryptionReady(
            [this, onRes, onErr]() { this->subscribeNotifications(onRes, onErr); }, onErr)) return;
    if (this->rejectIfCallbackQueueFull(onErr)) return;
    std::shared_ptr<NWCResponseCallback<NotificationResponse>> callback(new NWCResponseCallback<NotificationResponse>());
    callback->onRes = onRes;
    callback->onErr = onErr;
    callback->timestampSeconds = Utils::unixTimeSeconds();
    callback->timeoutSeconds = NWC_INFINITE_TIMEOUT; // notification subscriptions don't timeout, they can come after any amount of time
    const NostrString notificationKindString = this->notificationKind == 23197
                                                   ? NWC_NIP44_NOTIFICATION_KIND
                                                   : NWC_NIP04_NOTIFICATION_KIND;
    callback->subId = pool->subscribeMany(
        {this->nwc.relay}, {{{"kinds", {notificationKindString}}, {"authors", {this->nwc.pubkey}}, {"#p", {this->accountPubKey}}}},
        [&](const NostrString &subId, SignedNostrEvent *event) {
            if (event->getKind() != this->notificationKind) return;
            if (!NostrString_equals(event->getPubkey(), this->nwc.pubkey))  return;
            if (!event->verify()) return;
            if (!hasSingleTagValue(event, "p", this->accountPubKey)) return;
            NostrString subRef = event->getSubId();
            const std::vector<std::shared_ptr<NWCResponseCallbackBase>> snapshot = this->callbacks;
            for (const std::shared_ptr<NWCResponseCallbackBase> &callback : snapshot) {
                if (NostrString_equals(callback->subId, subRef)) {
                    callback->call(&this->nip47, event);
                    break;
                }
            }
        },
        [&](const String &subId, const String &reason) { Utils::log("NWC: closed notification subscription: " + reason); }, [&](const String &subId) { Utils::log("NWC: notification EOS"); });
    if (NostrString_length(callback->subId) == 0) {
        if (onErr) onErr("QUEUE_FULL", "Relay queue is full");
        return;
    }
    this->callbacks.push_back(callback);
}

void NWC::payInvoice(NostrString invoice, MilliSats amount, std::function<void(PayInvoiceResponse)> onRes, std::function<void(NostrString, NostrString)> onErr) {
    if (!this->validConnection) {
        if (onErr) onErr("INVALID_CONNECTION", "Invalid NWC connection string");
        return;
    }
    if (!isOptionalMilliSatsValid(amount)) {
        if (onErr) onErr("INVALID_AMOUNT", "Invalid millisatoshi amount");
        return;
    }
    if (this->deferUntilEncryptionReady(
            [this, invoice, amount, onRes, onErr]() { this->payInvoice(invoice, amount, onRes, onErr); }, onErr)) return;
    if (this->rejectIfCallbackQueueFull(onErr)) return;
    SignedNostrEvent ev = this->nip47.payInvoice(invoice, amount);
    std::shared_ptr<NWCResponseCallback<PayInvoiceResponse>> callback(new NWCResponseCallback<PayInvoiceResponse>());
    callback->onRes = onRes;
    callback->onErr = onErr;
    callback->timestampSeconds = Utils::unixTimeSeconds();
    callback->eventId = ev.getId();
    callback->n = 1;
    callback->subId = this->sendEvent(&ev, onErr);
    if (NostrString_length(callback->subId) == 0) return;
    this->callbacks.push_back(callback);
}

void NWC::multiPayInvoice(std::initializer_list<Invoice> invoices, std::function<void(MultiPayInvoiceResponse)> onRes, std::function<void(NostrString, NostrString)> onErr) {
    this->multiPayInvoice(std::vector<Invoice>(invoices), onRes, onErr);
}

void NWC::multiPayInvoice(const std::vector<Invoice> &invoices, std::function<void(MultiPayInvoiceResponse)> onRes,
                          std::function<void(NostrString, NostrString)> onErr) {
    if (!this->validConnection) {
        if (onErr) onErr("INVALID_CONNECTION", "Invalid NWC connection string");
        return;
    }
    for (const Invoice &invoice : invoices) {
        if (!isRequiredMilliSatsValid(invoice.amount)) {
            if (onErr) onErr("INVALID_AMOUNT", "Invalid millisatoshi amount");
            return;
        }
    }
    if (this->deferUntilEncryptionReady(
            [this, invoices, onRes, onErr]() { this->multiPayInvoice(invoices, onRes, onErr); }, onErr)) return;
    if (this->rejectIfCallbackQueueFull(onErr)) return;
    SignedNostrEvent ev = this->nip47.multiPayInvoice(invoices);
    std::shared_ptr<NWCResponseCallback<MultiPayInvoiceResponse>> callback(new NWCResponseCallback<MultiPayInvoiceResponse>());
    callback->onRes = onRes;
    callback->onErr = onErr;
    callback->timestampSeconds = Utils::unixTimeSeconds();
    callback->eventId = ev.getId();
    callback->n = invoices.size();
    callback->subId = this->sendEvent(&ev, onErr);
    if (NostrString_length(callback->subId) == 0) return;
    this->callbacks.push_back(callback);
}

void NWC::payKeySend(NostrString pubkey, MilliSats amount, NostrString preimage, std::initializer_list<TLVRecords> tlv, std::function<void(PayKeySendResponse)> onRes,
                     std::function<void(NostrString, NostrString)> onErr) {
    this->payKeySend(pubkey, amount, preimage, std::vector<TLVRecords>(tlv), onRes, onErr);
}

void NWC::payKeySend(NostrString pubkey, MilliSats amount, NostrString preimage,
                     const std::vector<TLVRecords> &tlv, std::function<void(PayKeySendResponse)> onRes,
                     std::function<void(NostrString, NostrString)> onErr) {
    if (!this->validConnection) {
        if (onErr) onErr("INVALID_CONNECTION", "Invalid NWC connection string");
        return;
    }
    if (!isRequiredMilliSatsValid(amount)) {
        if (onErr) onErr("INVALID_AMOUNT", "Invalid millisatoshi amount");
        return;
    }
    if (this->deferUntilEncryptionReady(
            [this, pubkey, amount, preimage, tlv, onRes, onErr]() {
                this->payKeySend(pubkey, amount, preimage, tlv, onRes, onErr);
            }, onErr)) return;
    if (this->rejectIfCallbackQueueFull(onErr)) return;
    SignedNostrEvent ev = this->nip47.payKeySend(pubkey, amount, preimage, tlv);
    std::shared_ptr<NWCResponseCallback<PayKeySendResponse>> callback(new NWCResponseCallback<PayKeySendResponse>());
    callback->onRes = onRes;
    callback->onErr = onErr;
    callback->timestampSeconds = Utils::unixTimeSeconds();
    callback->eventId = ev.getId();
    callback->n = 1;
    callback->subId = this->sendEvent(&ev, onErr);
    if (NostrString_length(callback->subId) == 0) return;
    this->callbacks.push_back(callback);
}

void NWC::multiPayKeySend(std::initializer_list<KeySend> keySends, std::function<void(MultiPayKeySendResponse)> onRes, std::function<void(NostrString, NostrString)> onErr) {
    this->multiPayKeySend(std::vector<KeySend>(keySends), onRes, onErr);
}

void NWC::multiPayKeySend(const std::vector<KeySend> &keySends, std::function<void(MultiPayKeySendResponse)> onRes,
                          std::function<void(NostrString, NostrString)> onErr) {
    if (!this->validConnection) {
        if (onErr) onErr("INVALID_CONNECTION", "Invalid NWC connection string");
        return;
    }
    for (const KeySend &keySend : keySends) {
        if (!isRequiredMilliSatsValid(keySend.amount)) {
            if (onErr) onErr("INVALID_AMOUNT", "Invalid millisatoshi amount");
            return;
        }
    }
    if (this->deferUntilEncryptionReady(
            [this, keySends, onRes, onErr]() { this->multiPayKeySend(keySends, onRes, onErr); }, onErr)) return;
    if (this->rejectIfCallbackQueueFull(onErr)) return;
    SignedNostrEvent ev = this->nip47.multiPayKeySend(keySends);
    std::shared_ptr<NWCResponseCallback<MultiPayKeySendResponse>> callback(new NWCResponseCallback<MultiPayKeySendResponse>());
    callback->onRes = onRes;
    callback->onErr = onErr;
    callback->timestampSeconds = Utils::unixTimeSeconds();
    callback->eventId = ev.getId();
    callback->n = keySends.size();
    callback->subId = this->sendEvent(&ev, onErr);
    if (NostrString_length(callback->subId) == 0) return;
    this->callbacks.push_back(callback);
}

void NWC::makeInvoice(MilliSats amount, NostrString description, NostrString descriptionHash, unsigned long expiry, std::function<void(MakeInvoiceResponse)> onRes,
                      std::function<void(NostrString, NostrString)> onErr) {
    if (!this->validConnection) {
        if (onErr) onErr("INVALID_CONNECTION", "Invalid NWC connection string");
        return;
    }
    if (!isRequiredMilliSatsValid(amount)) {
        if (onErr) onErr("INVALID_AMOUNT", "Invalid millisatoshi amount");
        return;
    }
    if (this->deferUntilEncryptionReady(
            [this, amount, description, descriptionHash, expiry, onRes, onErr]() {
                this->makeInvoice(amount, description, descriptionHash, expiry, onRes, onErr);
            }, onErr)) return;
    if (this->rejectIfCallbackQueueFull(onErr)) return;
    SignedNostrEvent ev = this->nip47.makeInvoice(amount, description, descriptionHash, expiry);
    std::shared_ptr<NWCResponseCallback<MakeInvoiceResponse>> callback(new NWCResponseCallback<MakeInvoiceResponse>());
    callback->onRes = onRes;
    callback->onErr = onErr;
    callback->timestampSeconds = Utils::unixTimeSeconds();
    callback->eventId = ev.getId();
    callback->n = 1;
    callback->subId = this->sendEvent(&ev, onErr);
    if (NostrString_length(callback->subId) == 0) return;
    this->callbacks.push_back(callback);
}

void NWC::lookUpPaymentHash(NostrString paymentHash, std::function<void(LookUpInvoiceResponse)> onRes, std::function<void(NostrString, NostrString)> onErr) {
    if (this->deferUntilEncryptionReady(
            [this, paymentHash, onRes, onErr]() { this->lookUpPaymentHash(paymentHash, onRes, onErr); }, onErr)) return;
    if (this->rejectIfCallbackQueueFull(onErr)) return;
    SignedNostrEvent ev = this->nip47.lookUpPaymentHash(paymentHash);
    std::shared_ptr<NWCResponseCallback<LookUpInvoiceResponse>> callback(new NWCResponseCallback<LookUpInvoiceResponse>());
    callback->onRes = onRes;
    callback->onErr = onErr;
    callback->timestampSeconds = Utils::unixTimeSeconds();
    callback->eventId = ev.getId();
    callback->n = 1;
    callback->subId = this->sendEvent(&ev, onErr);
    if (NostrString_length(callback->subId) == 0) return;
    this->callbacks.push_back(callback);
}

void NWC::lookUpInvoice(NostrString invoice, std::function<void(LookUpInvoiceResponse)> onRes, std::function<void(NostrString, NostrString)> onErr) {
    if (this->deferUntilEncryptionReady(
            [this, invoice, onRes, onErr]() { this->lookUpInvoice(invoice, onRes, onErr); }, onErr)) return;
    if (this->rejectIfCallbackQueueFull(onErr)) return;
    SignedNostrEvent ev = this->nip47.lookUpInvoice(invoice);
    std::shared_ptr<NWCResponseCallback<LookUpInvoiceResponse>> callback(new NWCResponseCallback<LookUpInvoiceResponse>());
    callback->onRes = onRes;
    callback->onErr = onErr;
    callback->timestampSeconds = Utils::unixTimeSeconds();
    callback->eventId = ev.getId();
    callback->n = 1;
    callback->subId = this->sendEvent(&ev, onErr);
    if (NostrString_length(callback->subId) == 0) return;
    this->callbacks.push_back(callback);
}

void NWC::listTransactions(unsigned long from, unsigned long until, int limit, int offset, bool unpaid, NostrString type, std::function<void(ListTransactionsResponse)> onRes,
                           std::function<void(NostrString, NostrString)> onErr) {
    if (this->deferUntilEncryptionReady(
            [this, from, until, limit, offset, unpaid, type, onRes, onErr]() {
                this->listTransactions(from, until, limit, offset, unpaid, type, onRes, onErr);
            }, onErr)) return;
    if (this->rejectIfCallbackQueueFull(onErr)) return;
    SignedNostrEvent ev = this->nip47.listTransactions(from, until, limit, offset, unpaid, type);
    std::shared_ptr<NWCResponseCallback<ListTransactionsResponse>> callback(new NWCResponseCallback<ListTransactionsResponse>());
    callback->onRes = onRes;
    callback->onErr = onErr;
    callback->timestampSeconds = Utils::unixTimeSeconds();
    callback->eventId = ev.getId();
    callback->n = 1;
    callback->subId = this->sendEvent(&ev, onErr);
    if (NostrString_length(callback->subId) == 0) return;
    this->callbacks.push_back(callback);
}

void NWC::getBalance(std::function<void(GetBalanceResponse)> onRes, std::function<void(NostrString, NostrString)> onErr) {
    if (this->deferUntilEncryptionReady(
            [this, onRes, onErr]() { this->getBalance(onRes, onErr); }, onErr)) return;
    if (this->rejectIfCallbackQueueFull(onErr)) return;
    SignedNostrEvent ev = this->nip47.getBalance();
    std::shared_ptr<NWCResponseCallback<GetBalanceResponse>> callback(new NWCResponseCallback<GetBalanceResponse>());
    callback->onRes = onRes;
    callback->onErr = onErr;
    callback->timestampSeconds = Utils::unixTimeSeconds();
    callback->eventId = ev.getId();
    callback->n = 1;
    callback->subId = this->sendEvent(&ev, onErr);
    if (NostrString_length(callback->subId) == 0) return;
    this->callbacks.push_back(callback);
}

void NWC::getInfo(std::function<void(GetInfoResponse)> onRes, std::function<void(NostrString, NostrString)> onErr) {
    if (this->deferUntilEncryptionReady(
            [this, onRes, onErr]() { this->getInfo(onRes, onErr); }, onErr)) return;
    if (this->rejectIfCallbackQueueFull(onErr)) return;
    SignedNostrEvent ev = this->nip47.getInfo();
    std::shared_ptr<NWCResponseCallback<GetInfoResponse>> callback(new NWCResponseCallback<GetInfoResponse>());
    callback->onRes = onRes;
    callback->onErr = onErr;
    callback->timestampSeconds = Utils::unixTimeSeconds();
    callback->eventId = ev.getId();
    callback->n = 1;
    callback->subId = this->sendEvent(&ev, onErr);
    if (NostrString_length(callback->subId) == 0) return;
    this->callbacks.push_back(callback);
}
