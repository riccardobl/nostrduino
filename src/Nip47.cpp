#include "Nip47.h"
#include "NostrUrl.h"
#include <cctype>
#include <cstring>

using namespace nostr;

namespace {

template <typename T>
void resetResponse(Nip47Response<T> &out) {
    out.errorCode = "";
    out.errorMessage = "";
    out.resultType = "";
    out.result = T{};
}

bool readStringField(JsonObjectConst object, const char *key, NostrString &out,
                     bool required = false) {
    JsonVariantConst value = object[key];
    if (value.isNull()) {
        out = "";
        return !required;
    }
    if (!value.is<const char *>()) return false;
    out = value.as<NostrString>();
    return !required || NostrString_length(out) > 0;
}

bool readUnsignedInteger(JsonVariantConst value, unsigned long long &out,
                         bool required = true) {
    if (value.isNull()) {
        out = 0;
        return !required;
    }
    if (!value.is<uint64_t>()) return false;
    out = value.as<uint64_t>();
    return true;
}

bool readMilliSats(JsonVariantConst value, MilliSats &out, bool required = true) {
    if (value.isNull()) {
        out = 0;
        return !required;
    }
    if (!value.is<int64_t>()) return false;
    out = value.as<int64_t>();
    return out >= 0;
}

bool isExactHexString(const NostrString &value, size_t byteCount) {
    if (NostrString_length(value) != static_cast<long>(byteCount * 2)) return false;
    std::vector<uint8_t> bytes(byteCount);
    return Utils::decodeHexExact(NostrString_toChars(value), bytes.data(), bytes.size());
}

bool readPreimage(JsonObjectConst object, NostrString &out, bool required) {
    if (!readStringField(object, "preimage", out, required)) return false;
    if (NostrString_length(out) == 0) return !required;
    return isExactHexString(out, 32);
}

bool copyMetadata(JsonVariantConst source, JsonDocument &destination) {
    destination.clear();
    if (source.isNull()) return true;
    if (!source.is<JsonObjectConst>()) return false;
    destination.set(source);
    return true;
}

bool readStringArray(JsonVariantConst source, std::vector<NostrString> &destination) {
    destination.clear();
    if (!source.is<JsonArrayConst>()) return false;
    for (JsonVariantConst value : source.as<JsonArrayConst>()) {
        if (!value.is<const char *>()) return false;
        destination.push_back(value.as<NostrString>());
    }
    return true;
}

bool strictQueryDecode(const NostrString &encoded, NostrString &decoded) {
    const long length = NostrString_length(encoded);
    if (length < 0) return false;
    for (long i = 0; i < length; ++i) {
        if (encoded[i] != '%') continue;
        if (i + 2 >= length ||
            !std::isxdigit(static_cast<unsigned char>(encoded[i + 1])) ||
            !std::isxdigit(static_cast<unsigned char>(encoded[i + 2]))) return false;
        i += 2;
    }
    decoded = NostrString_urlDecode(encoded);
    if (length > 0 && NostrString_length(decoded) == 0) return false;
    for (long i = 0; i < NostrString_length(decoded); ++i) {
        const unsigned char value = static_cast<unsigned char>(decoded[i]);
        if (value == 0 || value < 0x20 || value == 0x7f) return false;
    }
    return true;
}

} // namespace

void Nip47::setEncryption(Nip47Encryption selectedEncryption, bool shouldIncludeEncryptionTag) {
    this->encryption = selectedEncryption;
    this->includeEncryptionTag = shouldIncludeEncryptionTag;
}

NostrString Nip47::encryptContent(NostrString content) {
    if (this->encryption == Nip47Encryption::Nip44V2) {
        return this->nip44.encrypt(this->userPrivKey, this->servicePubKey, content);
    }
    return this->nip04.encrypt(this->userPrivKey, this->servicePubKey, content);
}

NostrString Nip47::decryptContent(NostrString content) {
    if (this->encryption == Nip47Encryption::Nip44V2) {
        return this->nip44.decrypt(this->userPrivKey, this->servicePubKey, content);
    }
    return this->nip04.decrypt(this->userPrivKey, this->servicePubKey, content);
}

bool Nip47::parseResponseEnvelope(SignedNostrEvent *response, const char *expectedResultType,
                                  JsonDocument &document, JsonObjectConst &result,
                                  NostrString &errorCode, NostrString &errorMessage) {
    if (response == nullptr || !response->verify()) return false;
    NostrString content = this->decryptContent(response->getContent());
    if (!Utils::jsonParse(&content, &document) || !document.is<JsonObject>()) return false;
    JsonObjectConst root = document.as<JsonObjectConst>();
    if (!root["result_type"].is<const char *>() ||
        std::strcmp(root["result_type"].as<const char *>(), expectedResultType) != 0) return false;

    JsonVariantConst errorValue = root["error"];
    JsonVariantConst resultValue = root["result"];
    if (!errorValue.isNull()) {
        if (!errorValue.is<JsonObjectConst>() || !resultValue.isNull()) return false;
        JsonObjectConst error = errorValue.as<JsonObjectConst>();
        if (!readStringField(error, "code", errorCode, true) ||
            !readStringField(error, "message", errorMessage, true)) {
            errorCode = "";
            errorMessage = "";
        }
        return false;
    }
    if (!resultValue.is<JsonObjectConst>()) return false;
    result = resultValue.as<JsonObjectConst>();
    return true;
}

SignedNostrEvent Nip47::createEvent(NostrString method, JsonDocument doc) {
    doc["method"] = method;
    NostrString contentString;
    Utils::jsonStringify(doc, &contentString);
    doc.clear();
    contentString = this->encryptContent(contentString);
    UnsignedNostrEvent event(23194, contentString, Utils::unixTimeSeconds());
    NostrEventTags *tags = event.getTags();
    if (this->includeEncryptionTag) {
        tags->addTag("encryption", {this->encryption == Nip47Encryption::Nip44V2 ? "nip44_v2" : "nip04"});
    }
    tags->addTag("p", {this->servicePubKey});
    return event.sign(this->userPrivKey);
}

SignedNostrEvent Nip47::payInvoice(NostrString invoice, MilliSats amount) {
    if (!isOptionalMilliSatsValid(amount)) return SignedNostrEvent();
    JsonDocument doc;
    JsonObject params = doc["params"].to<JsonObject>();
    params["invoice"] = invoice;
    if (amount > 0) {
        params["amount"] = amount;
    }
    return this->createEvent("pay_invoice", doc);
}

SignedNostrEvent Nip47::makeInvoice(MilliSats amount, NostrString description, NostrString descriptionHash, unsigned long expiry) {
    if (!isRequiredMilliSatsValid(amount)) return SignedNostrEvent();
    JsonDocument doc;
    JsonObject params = doc["params"].to<JsonObject>();
    params["amount"] = amount;
    if (NostrString_length(description) > 0) {
        params["description"] = description;
    }
    if (NostrString_length(descriptionHash) > 0) {
        params["description_hash"] = descriptionHash;
    }
    if (expiry > 0) {
        params["expiry"] = expiry;
    }
    return this->createEvent("make_invoice", doc);
}

SignedNostrEvent Nip47::lookUpPaymentHash(NostrString payment_hash) {
    JsonDocument doc;
    JsonObject params = doc["params"].to<JsonObject>();
    params["payment_hash"] = payment_hash;
    return this->createEvent("lookup_invoice", doc);
}

SignedNostrEvent Nip47::lookUpInvoice(NostrString invoice) {
    JsonDocument doc;
    JsonObject params = doc["params"].to<JsonObject>();
    params["invoice"] = invoice;
    return this->createEvent("lookup_invoice", doc);
}

SignedNostrEvent Nip47::getBalance() {
    JsonDocument doc;
    JsonObject params = doc["params"].to<JsonObject>();
    return this->createEvent("get_balance", doc);
}

SignedNostrEvent Nip47::getInfo() {
    JsonDocument doc;
    JsonObject params = doc["params"].to<JsonObject>();
    return this->createEvent("get_info", doc);
}

SignedNostrEvent Nip47::multiPayInvoice(std::initializer_list<Invoice> invoices) {
    return this->multiPayInvoice(std::vector<Invoice>(invoices));
}

SignedNostrEvent Nip47::multiPayInvoice(const std::vector<Invoice> &invoices) {
    for (const Invoice &invoice : invoices) {
        if (!isRequiredMilliSatsValid(invoice.amount)) return SignedNostrEvent();
    }
    JsonDocument doc;
    JsonObject params = doc["params"].to<JsonObject>();
    JsonArray iv = params["invoices"].to<JsonArray>();
    for (auto invoice : invoices) {
        JsonObject record = iv.add<JsonObject>();
        record["invoice"] = invoice.invoice;
        record["amount"] = invoice.amount;
    }
    return this->createEvent("multi_pay_invoice", doc);
}

SignedNostrEvent Nip47::payKeySend(NostrString pubkey, MilliSats amount, NostrString preimage, std::initializer_list<TLVRecords> tlv) {
    return this->payKeySend(pubkey, amount, preimage, std::vector<TLVRecords>(tlv));
}

SignedNostrEvent Nip47::payKeySend(NostrString pubkey, MilliSats amount, NostrString preimage,
                                    const std::vector<TLVRecords> &tlv) {
    if (!isRequiredMilliSatsValid(amount)) return SignedNostrEvent();
    JsonDocument doc;
    JsonObject params = doc["params"].to<JsonObject>();
    params["pubkey"] = pubkey;
    params["amount"] = amount;
    if (NostrString_length(preimage) > 0) {
        params["preimage"] = preimage;
    }
    if (tlv.size() > 0) {
        JsonArray tlvArray = params["tlv_records"].to<JsonArray>();
        for (auto record : tlv) {
            JsonObject tlvRecord = tlvArray.add<JsonObject>();
            tlvRecord["type"] = record.type;
            tlvRecord["value"] = record.value;
        }
    }
    return this->createEvent("pay_keysend", doc);
}

SignedNostrEvent Nip47::multiPayKeySend(std::initializer_list<KeySend> keySends) {
    return this->multiPayKeySend(std::vector<KeySend>(keySends));
}

SignedNostrEvent Nip47::multiPayKeySend(const std::vector<KeySend> &keySends) {
    for (const KeySend &keySend : keySends) {
        if (!isRequiredMilliSatsValid(keySend.amount)) return SignedNostrEvent();
    }
    JsonDocument doc;
    JsonObject params = doc["params"].to<JsonObject>();
    JsonArray ks = params["key_sends"].to<JsonArray>();
    for (auto keySend : keySends) {
        JsonObject record = ks.add<JsonObject>();
        record["pubkey"] = keySend.pubkey;
        record["amount"] = keySend.amount;
        if (NostrString_length(keySend.preimage) > 0) {
            record["preimage"] = keySend.preimage;
        }
        if (keySend.tlv.size() > 0) {
            JsonArray tlvArray = record["tlv_records"].to<JsonArray>();
            for (auto tlvRecord : keySend.tlv) {
                JsonObject tlv = tlvArray.add<JsonObject>();
                tlv["type"] = tlvRecord.type;
                tlv["value"] = tlvRecord.value;
            }
        }
    }
    return this->createEvent("multi_pay_keysend", doc);
}

SignedNostrEvent Nip47::listTransactions(unsigned long from, unsigned long until, int limit, int offset, bool unpaid, NostrString type) {
    JsonDocument doc;
    JsonObject params = doc["params"].to<JsonObject>();
    if (from > 0) {
        params["from"] = from;
    }
    if (until > 0) {
        params["until"] = until;
    }
    if (limit > 0) {
        params["limit"] = limit;
    }
    if (offset > 0) {
        params["offset"] = offset;
    }
    params["unpaid"] = unpaid;
    if (NostrString_length(type) > 0) {
        params["type"] = type;
    }
    return this->createEvent("list_transactions", doc);
}

void Nip47::parseResponse(SignedNostrEvent *response, Nip47Response<PayInvoiceResponse> &out) {
    resetResponse(out);
    Utils::log("NWC: received pay_invoice response");
    JsonDocument document;
    JsonObjectConst result;
    NostrString preimage;
    if (this->parseResponseEnvelope(response, "pay_invoice", document, result,
                                    out.errorCode, out.errorMessage) &&
        readPreimage(result, preimage, true)) {
        out.result.preimage = preimage;
        return;
    }
    if (NostrString_length(out.errorCode) == 0) {
        out.errorCode = "OTHER";
        out.errorMessage = "Invalid Event";
    }
}

void Nip47::parseResponse(SignedNostrEvent *response, Nip47Response<MultiPayInvoiceResponse> &out) {
    resetResponse(out);
    Utils::log("NWC: received multi_pay_invoice response");
    JsonDocument document;
    JsonObjectConst result;
    NostrString preimage;
    const std::vector<NostrString> &dTags = response != nullptr
                                                ? response->getTags()->getTag("d")
                                                : NostrEventTags::emptyTags;
    if (this->parseResponseEnvelope(response, "multi_pay_invoice", document, result,
                                    out.errorCode, out.errorMessage) &&
        dTags.size() == 1 && NostrString_length(dTags[0]) > 0 &&
        readPreimage(result, preimage, true)) {
        out.result.preimage = preimage;
        out.result.d = dTags[0];
        return;
    }
    if (NostrString_length(out.errorCode) == 0) {
        out.errorCode = "OTHER";
        out.errorMessage = "Invalid Event";
    }
}

void Nip47::parseResponse(SignedNostrEvent *response, Nip47Response<PayKeySendResponse> &out) {
    resetResponse(out);
    Utils::log("NWC: received pay_keysend response");
    JsonDocument document;
    JsonObjectConst result;
    NostrString preimage;
    if (this->parseResponseEnvelope(response, "pay_keysend", document, result,
                                    out.errorCode, out.errorMessage) &&
        readPreimage(result, preimage, true)) {
        out.result.preimage = preimage;
        return;
    }
    if (NostrString_length(out.errorCode) == 0) {
        out.errorCode = "OTHER";
        out.errorMessage = "Invalid Event";
    }
}

void Nip47::parseResponse(SignedNostrEvent *response, Nip47Response<MultiPayKeySendResponse> &out) {
    resetResponse(out);
    Utils::log("NWC: received multi_pay_keysend response");
    JsonDocument document;
    JsonObjectConst result;
    NostrString preimage;
    const std::vector<NostrString> &dTags = response != nullptr
                                                ? response->getTags()->getTag("d")
                                                : NostrEventTags::emptyTags;
    if (this->parseResponseEnvelope(response, "multi_pay_keysend", document, result,
                                    out.errorCode, out.errorMessage) &&
        dTags.size() == 1 && NostrString_length(dTags[0]) > 0 &&
        readPreimage(result, preimage, true)) {
        out.result.preimage = preimage;
        out.result.d = dTags[0];
        return;
    }
    if (NostrString_length(out.errorCode) == 0) {
        out.errorCode = "OTHER";
        out.errorMessage = "Invalid Event";
    }
}

void Nip47::parseResponse(SignedNostrEvent *response, Nip47Response<MakeInvoiceResponse> &out) {
    resetResponse(out);
    Utils::log("NWC: received make_invoice response");
    JsonDocument document;
    JsonObjectConst result;
    MakeInvoiceResponse parsed{};
    if (this->parseResponseEnvelope(response, "make_invoice", document, result,
                                    out.errorCode, out.errorMessage) &&
        readStringField(result, "type", parsed.type, true) &&
        readStringField(result, "invoice", parsed.invoice, true) &&
        readStringField(result, "description", parsed.description) &&
        readStringField(result, "description_hash", parsed.descriptionHash) &&
        readPreimage(result, parsed.preimage, false) &&
        readStringField(result, "payment_hash", parsed.paymentHash, true) &&
        readMilliSats(result["amount"], parsed.amount) &&
        readMilliSats(result["fees_paid"], parsed.feesPaid, false) &&
        readUnsignedInteger(result["created_at"], parsed.createdAt) &&
        readUnsignedInteger(result["expires_at"], parsed.expiresAt) &&
        copyMetadata(result["metadata"], parsed.metadata)) {
        out.result = parsed;
        return;
    }
    if (NostrString_length(out.errorCode) == 0) {
        out.errorCode = "OTHER";
        out.errorMessage = "Invalid Event";
    }
}

void Nip47::parseResponse(SignedNostrEvent *response, Nip47Response<LookUpInvoiceResponse> &out) {
    resetResponse(out);
    Utils::log("NWC: received lookup_invoice response");
    JsonDocument document;
    JsonObjectConst result;
    LookUpInvoiceResponse parsed{};
    if (this->parseResponseEnvelope(response, "lookup_invoice", document, result,
                                    out.errorCode, out.errorMessage) &&
        readStringField(result, "type", parsed.type, true) &&
        readStringField(result, "invoice", parsed.invoice) &&
        readStringField(result, "description", parsed.description) &&
        readStringField(result, "description_hash", parsed.descriptionHash) &&
        readPreimage(result, parsed.preimage, false) &&
        readStringField(result, "payment_hash", parsed.paymentHash, true) &&
        readMilliSats(result["amount"], parsed.amount) &&
        readMilliSats(result["fees_paid"], parsed.feesPaid, false) &&
        readUnsignedInteger(result["created_at"], parsed.createdAt) &&
        readUnsignedInteger(result["expires_at"], parsed.expiresAt, false) &&
        readUnsignedInteger(result["settled_at"], parsed.settledAt, false) &&
        copyMetadata(result["metadata"], parsed.metadata)) {
        out.result = parsed;
        return;
    }
    if (NostrString_length(out.errorCode) == 0) {
        out.errorCode = "OTHER";
        out.errorMessage = "Invalid Event";
    }
}

void Nip47::parseResponse(SignedNostrEvent *response, Nip47Response<ListTransactionsResponse> &out) {
    resetResponse(out);
    Utils::log("NWC: received list_transactions response");
    JsonDocument document;
    JsonObjectConst result;
    std::vector<Transaction> parsedTransactions;
    if (this->parseResponseEnvelope(response, "list_transactions", document, result,
                                    out.errorCode, out.errorMessage) &&
        result["transactions"].is<JsonArrayConst>()) {
        bool valid = true;
        for (JsonVariantConst value : result["transactions"].as<JsonArrayConst>()) {
            if (!value.is<JsonObjectConst>()) {
                valid = false;
                break;
            }
            JsonObjectConst transactionJson = value.as<JsonObjectConst>();
            Transaction transaction{};
            if (!readStringField(transactionJson, "type", transaction.type, true) ||
                !readStringField(transactionJson, "invoice", transaction.invoice) ||
                !readStringField(transactionJson, "description", transaction.description) ||
                !readStringField(transactionJson, "description_hash", transaction.descriptionHash) ||
                !readPreimage(transactionJson, transaction.preimage, false) ||
                !readStringField(transactionJson, "payment_hash", transaction.paymentHash, true) ||
                !readMilliSats(transactionJson["amount"], transaction.amount) ||
                !readMilliSats(transactionJson["fees_paid"], transaction.feesPaid, false) ||
                !readUnsignedInteger(transactionJson["created_at"], transaction.createdAt) ||
                !readUnsignedInteger(transactionJson["expires_at"], transaction.expiresAt, false) ||
                !readUnsignedInteger(transactionJson["settled_at"], transaction.settledAt, false) ||
                !copyMetadata(transactionJson["metadata"], transaction.metadata)) {
                valid = false;
                break;
            }
            parsedTransactions.push_back(transaction);
        }
        if (valid) {
            out.result.transactions = parsedTransactions;
            return;
        }
    }
    if (NostrString_length(out.errorCode) == 0) {
        out.errorCode = "OTHER";
        out.errorMessage = "Invalid Event";
    }
}

void Nip47::parseResponse(SignedNostrEvent *response, Nip47Response<GetBalanceResponse> &out) {
    resetResponse(out);
    Utils::log("NWC: received get_balance response");
    JsonDocument document;
    JsonObjectConst result;
    MilliSats balance = 0;
    if (this->parseResponseEnvelope(response, "get_balance", document, result,
                                    out.errorCode, out.errorMessage) &&
        readMilliSats(result["balance"], balance)) {
        out.result.balance = balance;
        return;
    }
    if (NostrString_length(out.errorCode) == 0) {
        out.errorCode = "OTHER";
        out.errorMessage = "Invalid Event";
    }
}

void Nip47::parseResponse(SignedNostrEvent *response, Nip47Response<GetInfoResponse> &out) {
    resetResponse(out);
    Utils::log("NWC: received get_info response");
    JsonDocument document;
    JsonObjectConst result;
    GetInfoResponse parsed{};
    if (this->parseResponseEnvelope(response, "get_info", document, result,
                                    out.errorCode, out.errorMessage) &&
        readStringField(result, "alias", parsed.alias) &&
        readStringField(result, "color", parsed.color) &&
        readStringField(result, "pubkey", parsed.pubkey, true) &&
        readStringField(result, "network", parsed.network, true) &&
        readUnsignedInteger(result["block_height"], parsed.blockHeight) &&
        readStringField(result, "block_hash", parsed.blockHash, true) &&
        readStringArray(result["methods"], parsed.methods) &&
        readStringArray(result["notifications"], parsed.notifications)) {
        out.result = parsed;
        return;
    }
    if (NostrString_length(out.errorCode) == 0) {
        out.errorCode = "OTHER";
        out.errorMessage = "Invalid Event";
    }
}

bool Nip47::parseNWC(NostrString nwc, NWCData &data) {
    const NostrString prefix = "nostr+walletconnect://";
    if (!NostrString_startsWith(nwc, NostrString_toChars(prefix))) return false;
    const NostrString withoutPrefix = NostrString_substring(nwc, NostrString_length(prefix));
    const int queryPos = NostrString_indexOf(withoutPrefix, "?");
    if (queryPos <= 0) return false;

    NWCData parsed;
    parsed.pubkey = NostrString_substring(withoutPrefix, 0, queryPos);
    uint8_t publicKey[32] = {};
    if (!Utils::decodeHexExact(parsed.pubkey, publicKey, sizeof(publicKey))) return false;

    const NostrString query = NostrString_substring(withoutPrefix, queryPos + 1);
    if (NostrString_length(query) == 0) return false;
    std::vector<NostrString> parameters;
    NostrString_split(query, '&', parameters);
    bool hasSecret = false;
    for (const NostrString &parameter : parameters) {
        const int equals = NostrString_indexOf(parameter, "=");
        if (equals <= 0 || NostrString_indexOf(NostrString_substring(parameter, equals + 1), "=") != -1) return false;
        const NostrString key = NostrString_substring(parameter, 0, equals);
        const NostrString encodedValue = NostrString_substring(parameter, equals + 1);
        NostrString value;
        if (!strictQueryDecode(encodedValue, value)) return false;
        if (key == "relay") {
            ParsedUrl relayUrl;
            if (!parseWebSocketUrl(value, relayUrl)) return false;
            bool duplicate = false;
            for (const NostrString &relay : parsed.relays) {
                if (NostrString_equals(relay, value)) {
                    duplicate = true;
                    break;
                }
            }
            if (!duplicate) parsed.relays.push_back(value);
        } else if (key == "secret") {
            if (hasSecret) return false;
            hasSecret = true;
            parsed.secret = value;
        }
    }
    if (!hasSecret || parsed.relays.empty()) return false;

    uint8_t secret[32] = {};
    uint8_t shared[32] = {};
    const bool validKeys = Utils::decodeHexExact(parsed.secret, secret, sizeof(secret)) &&
                           Utils::ecdh(parsed.secret, parsed.pubkey, shared);
    for (size_t i = 0; i < sizeof(secret); ++i) secret[i] = 0;
    for (size_t i = 0; i < sizeof(shared); ++i) shared[i] = 0;
    if (!validKeys) return false;

    parsed.relay = parsed.relays.front();
    data = parsed;
    return true;
}

void Nip47::parseResponse(SignedNostrEvent *response, Nip47Response<NotificationResponse> &out) {
    resetResponse(out);
    Utils::log("NWC: received notification response");
    if (response != nullptr && response->verify()) {
        NostrString content = this->decryptContent(response->getContent());
        JsonDocument document;
        if (Utils::jsonParse(&content, &document) && document.is<JsonObject>()) {
            JsonObjectConst root = document.as<JsonObjectConst>();
            JsonVariantConst errorValue = root["error"];
            if (!errorValue.isNull()) {
                if (errorValue.is<JsonObjectConst>() && root["notification"].isNull()) {
                    JsonObjectConst error = errorValue.as<JsonObjectConst>();
                    if (readStringField(error, "code", out.errorCode, true) &&
                        readStringField(error, "message", out.errorMessage, true)) return;
                }
                out.errorCode = "";
                out.errorMessage = "";
            } else if (root["notification_type"].is<const char *>() &&
                       root["notification"].is<JsonObjectConst>()) {
                NostrString notificationType = root["notification_type"].as<NostrString>();
                JsonObjectConst notification = root["notification"].as<JsonObjectConst>();
                Nip47Notification parsed{};
                parsed.notificationType = notificationType;
                if (NostrString_length(notificationType) > 0 &&
                    readStringField(notification, "type", parsed.type, true) &&
                    readStringField(notification, "invoice", parsed.invoice) &&
                    readStringField(notification, "description", parsed.description) &&
                    readStringField(notification, "description_hash", parsed.descriptionHash) &&
                    readPreimage(notification, parsed.preimage, false) &&
                    readStringField(notification, "payment_hash", parsed.paymentHash) &&
                    readMilliSats(notification["amount"], parsed.amount) &&
                    readMilliSats(notification["fees_paid"], parsed.feesPaid, false) &&
                    readUnsignedInteger(notification["created_at"], parsed.createdAt) &&
                    readUnsignedInteger(notification["settled_at"], parsed.settledAt, false)) {
                    out.resultType = notificationType;
                    out.result.notification = parsed;
                    return;
                }
            }
        }
    }
    if (NostrString_length(out.errorCode) == 0) {
        out.errorCode = "OTHER";
        out.errorMessage = "Invalid Notification Event";
    }
}
