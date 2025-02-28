#include "Nip47.h"

using namespace nostr;

SignedNostrEvent Nip47::createEvent(NostrString method, JsonDocument doc) {
    doc["method"] = method;
    NostrString contentString;
    Utils::jsonStringify(doc, &contentString);
    doc.clear();
    contentString = this->nip04.encrypt(this->userPrivKey, this->servicePubKey, contentString);
    UnsignedNostrEvent event(23194, contentString, Utils::unixTimeSeconds());
    NostrEventTags *tags = event.getTags();
    tags->addTag("p", {this->servicePubKey});
    return event.sign(this->userPrivKey);
}

SignedNostrEvent Nip47::payInvoice(NostrString invoice, unsigned long amount) {
    JsonDocument doc;
    JsonObject params = doc["params"].to<JsonObject>();
    params["invoice"] = invoice;
    if (amount > 0) {
        params["amount"] = amount;
    }
    return this->createEvent("pay_invoice", doc);
}

SignedNostrEvent Nip47::makeInvoice(unsigned long amount, NostrString description, NostrString descriptionHash, unsigned long expiry) {
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

SignedNostrEvent Nip47::payKeySend(NostrString pubkey, unsigned long amount, NostrString preimage, std::initializer_list<TLVRecords> tlv) {
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
    return this->createEvent("pay_key_send", doc);
}

SignedNostrEvent Nip47::multiPayKeySend(std::initializer_list<KeySend> keySends) {
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
    return this->createEvent("multi_pay_key_send", doc);
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
    out.errorCode = "";
    out.errorMessage = "";
    Utils::log("NWC: received pay_invoice response");
    if (response->verify()) {
        NostrString content = this->nip04.decrypt(this->userPrivKey, this->servicePubKey, response->getContent());
        JsonDocument doc;
        Utils::jsonParse(&content, &doc);
        JsonObject error = doc["error"];
        if (!error.isNull()) {
            out.errorCode = error["code"].as<NostrString>();
            out.errorMessage = error["message"].as<NostrString>();
        } else {
            if (doc.containsKey("result") && doc.containsKey("result_type")) {
                NostrString resultType = doc["result_type"].as<NostrString>();
                JsonObject data = doc["result"];
                if (NostrString_equals(resultType, "pay_invoice")) {
                    out.result.preimage = data["preimage"].as<NostrString>();
                    return;
                }
            }
        }
    }
    if (NostrString_length(out.errorCode) == 0) {
        out.errorCode = "OTHER";
        out.errorMessage = "Invalid Event";
    }
}

void Nip47::parseResponse(SignedNostrEvent *response, Nip47Response<MultiPayInvoiceResponse> &out) {
    out.errorCode = "";
    out.errorMessage = "";
    Utils::log("NWC: received multi_pay_invoice response");
    if (response->verify()) {
        NostrString content = this->nip04.decrypt(this->userPrivKey, this->servicePubKey, response->getContent());
        JsonDocument doc;
        Utils::jsonParse(&content, &doc);
        JsonObject error = doc["error"];
        if (!error.isNull()) {
            out.errorCode = error["code"].as<NostrString>();
            out.errorMessage = error["message"].as<NostrString>();
        } else {
            if (doc.containsKey("result") && doc.containsKey("result_type")) {
                NostrString resultType = doc["result_type"].as<NostrString>();
                JsonObject data = doc["result"];
                if (NostrString_equals(resultType, "multi_pay_invoice")) {
                    out.result.preimage = data["preimage"].as<NostrString>();
                    out.result.d = response->getTags()->getTag("d")[0];
                    return;
                }
            }
        }
    }
    if (NostrString_length(out.errorCode) == 0) {
        out.errorCode = "OTHER";
        out.errorMessage = "Invalid Event";
    }
}

void Nip47::parseResponse(SignedNostrEvent *response, Nip47Response<PayKeySendResponse> &out) {
    out.errorCode = "";
    out.errorMessage = "";
    Utils::log("NWC: received pay_keysend response");
    if (response->verify()) {
        NostrString content = this->nip04.decrypt(this->userPrivKey, this->servicePubKey, response->getContent());
        JsonDocument doc;
        Utils::jsonParse(&content, &doc);
        JsonObject error = doc["error"];
        if (!error.isNull()) {
            out.errorCode = error["code"].as<NostrString>();
            out.errorMessage = error["message"].as<NostrString>();
        } else {
            if (doc.containsKey("result") && doc.containsKey("result_type")) {
                NostrString resultType = doc["result_type"].as<NostrString>();
                JsonObject data = doc["result"];
                if (NostrString_equals(resultType, "pay_keysend")) {
                    out.result.preimage = data["preimage"].as<NostrString>();
                    return;
                }
            }
        }
    }
    if (NostrString_length(out.errorCode) == 0) {
        out.errorCode = "OTHER";
        out.errorMessage = "Invalid Event";
    }
}

void Nip47::parseResponse(SignedNostrEvent *response, Nip47Response<MultiPayKeySendResponse> &out) {
    out.errorCode = "";
    out.errorMessage = "";
    Utils::log("NWC: received multi_pay_keysend response");
    if (response->verify()) {
        NostrString content = this->nip04.decrypt(this->userPrivKey, this->servicePubKey, response->getContent());
        JsonDocument doc;
        Utils::jsonParse(&content, &doc);
        JsonObject error = doc["error"];
        if (!error.isNull()) {
            out.errorCode = error["code"].as<NostrString>();
            out.errorMessage = error["message"].as<NostrString>();
        } else {
            if (doc.containsKey("result") && doc.containsKey("result_type")) {
                NostrString resultType = doc["result_type"].as<NostrString>();
                JsonObject data = doc["result"];
                if (NostrString_equals(resultType, "multi_pay_keysend")) {
                    out.result.preimage = data["preimage"].as<NostrString>();
                    out.result.d = response->getTags()->getTag("d")[0];
                    return;
                }
            }
        }
    }
    if (NostrString_length(out.errorCode) == 0) {
        out.errorCode = "OTHER";
        out.errorMessage = "Invalid Event";
    }
}

void Nip47::parseResponse(SignedNostrEvent *response, Nip47Response<MakeInvoiceResponse> &out) {
    out.errorCode = "";
    out.errorMessage = "";
    Utils::log("NWC: received make_invoice response");
    if (response->verify()) {
        NostrString content = this->nip04.decrypt(this->userPrivKey, this->servicePubKey, response->getContent());
        JsonDocument doc;
        Utils::jsonParse(&content, &doc);
        JsonObject error = doc["error"];
        if (!error.isNull()) {
            out.errorCode = error["code"].as<NostrString>();
            out.errorMessage = error["message"].as<NostrString>();
        } else {
            if (doc.containsKey("result") && doc.containsKey("result_type")) {
                NostrString resultType = doc["result_type"].as<NostrString>();
                JsonObject data = doc["result"];
                if (NostrString_equals(resultType, "make_invoice")) {
                    out.result.type = data["type"].as<NostrString>();
                    out.result.invoice = data["invoice"].as<NostrString>();
                    out.result.description = data["description"].as<NostrString>();
                    out.result.descriptionHash = data["description_hash"].as<NostrString>();
                    out.result.preimage = data["preimage"].as<NostrString>();
                    out.result.paymentHash = data["payment_hash"].as<NostrString>();
                    out.result.amount = data["amount"].as<unsigned long long>();
                    out.result.feesPaid = data["fees_paid"].as<unsigned long long>();
                    out.result.createdAt = data["created_at"].as<unsigned long long>();
                    out.result.expiresAt = data["expires_at"].as<unsigned long long>();
                    JsonDocument metadataDoc;
                    JsonObject metadataObject = metadataDoc.as<JsonObject>();
                    for (JsonPair kv : data["metadata"].as<JsonObject>()) {
                        metadataObject[kv.key().c_str()] = kv.value().as<NostrString>();
                    }
                    out.result.metadata = metadataObject;
                    return;
                }
            }
        }
    }
    if (NostrString_length(out.errorCode) == 0) {
        out.errorCode = "OTHER";
        out.errorMessage = "Invalid Event";
    }
}

void Nip47::parseResponse(SignedNostrEvent *response, Nip47Response<LookUpInvoiceResponse> &out) {
    out.errorCode = "";
    out.errorMessage = "";
    Utils::log("NWC: received lookup_invoice response");
    if (response->verify()) {
        NostrString content = this->nip04.decrypt(this->userPrivKey, this->servicePubKey, response->getContent());
        JsonDocument doc;
        Utils::jsonParse(&content, &doc);
        JsonObject error = doc["error"];
        if (!error.isNull()) {
            out.errorCode = error["code"].as<NostrString>();
            out.errorMessage = error["message"].as<NostrString>();
        } else {
            if (doc.containsKey("result") && doc.containsKey("result_type")) {
                NostrString resultType = doc["result_type"].as<NostrString>();
                JsonObject data = doc["result"];
                if (NostrString_equals(resultType, "lookup_invoice")) {
                    out.result.type = data["type"].as<NostrString>();
                    out.result.invoice = data["invoice"].as<NostrString>();
                    out.result.description = data["description"].as<NostrString>();
                    out.result.descriptionHash = data["description_hash"].as<NostrString>();
                    out.result.preimage = data["preimage"].as<NostrString>();
                    out.result.paymentHash = data["payment_hash"].as<NostrString>();
                    out.result.amount = data["amount"].as<unsigned long long>();
                    out.result.feesPaid = data["fees_paid"].as<unsigned long long>();
                    out.result.createdAt = data["created_at"].as<unsigned long long>();
                    out.result.expiresAt = data["expires_at"].as<unsigned long long>();
                    out.result.settledAt = data["settled_at"].as<unsigned long long>();
                    JsonDocument metadataDoc;
                    JsonObject metadataObject = metadataDoc.as<JsonObject>();
                    for (JsonPair kv : data["metadata"].as<JsonObject>()) {
                        metadataObject[kv.key().c_str()] = kv.value().as<NostrString>();
                    }
                    out.result.metadata = metadataObject;
                    return;
                }
            }
        }
    }
    if (NostrString_length(out.errorCode) == 0) {
        out.errorCode = "OTHER";
        out.errorMessage = "Invalid Event";
    }
}

void Nip47::parseResponse(SignedNostrEvent *response, Nip47Response<ListTransactionsResponse> &out) {
    out.errorCode = "";
    out.errorMessage = "";
    Utils::log("NWC: received list_transactions response");
    if (response->verify()) {
        NostrString content = this->nip04.decrypt(this->userPrivKey, this->servicePubKey, response->getContent());
        JsonDocument doc;
        Utils::jsonParse(&content, &doc);
        JsonObject error = doc["error"];
        if (!error.isNull()) {
            out.errorCode = error["code"].as<NostrString>();
            out.errorMessage = error["message"].as<NostrString>();
        } else {
            if (doc.containsKey("result") && doc.containsKey("result_type")) {
                NostrString resultType = doc["result_type"].as<NostrString>();
                JsonObject data = doc["result"];
                if (NostrString_equals(resultType, "list_transactions")) {
                    for (JsonObject kv : data["transactions"].as<JsonArray>()) {
                        Transaction transaction;
                        transaction.type = kv["type"].as<NostrString>();
                        transaction.invoice = kv["invoice"].as<NostrString>();
                        transaction.description = kv["description"].as<NostrString>();
                        transaction.descriptionHash = kv["description_hash"].as<NostrString>();
                        transaction.preimage = kv["preimage"].as<NostrString>();
                        transaction.paymentHash = kv["payment_hash"].as<NostrString>();
                        transaction.amount = kv["amount"].as<unsigned long long>();
                        transaction.feesPaid = kv["fees_paid"].as<unsigned long long>();
                        transaction.createdAt = kv["created_at"].as<unsigned long long>();
                        transaction.expiresAt = kv["expires_at"].as<unsigned long long>();
                        transaction.settledAt = kv["settled_at"].as<unsigned long long>();
                        JsonDocument metadataDoc;
                        JsonObject metadataObject = metadataDoc.as<JsonObject>();
                        for (JsonPair kv : kv["metadata"].as<JsonObject>()) {
                            metadataObject[kv.key().c_str()] = kv.value().as<NostrString>();
                        }
                        transaction.metadata = metadataObject;
                        out.result.transactions.push_back(transaction);
                    }
                    return;
                }
            }
        }
    }
    if (NostrString_length(out.errorCode) == 0) {
        out.errorCode = "OTHER";
        out.errorMessage = "Invalid Event";
    }
}

void Nip47::parseResponse(SignedNostrEvent *response, Nip47Response<GetBalanceResponse> &out) {
    out.errorCode = "";
    out.errorMessage = "";
    Utils::log("NWC: received get_balance response");
    if (response->verify()) {
        NostrString content = this->nip04.decrypt(this->userPrivKey, this->servicePubKey, response->getContent());
        JsonDocument doc;
        Utils::jsonParse(&content, &doc);
        JsonObject error = doc["error"];
        if (!error.isNull()) {
            out.errorCode = error["code"].as<NostrString>();
            out.errorMessage = error["message"].as<NostrString>();
        } else {
            if (doc.containsKey("result") && doc.containsKey("result_type")) {
                NostrString resultType = doc["result_type"].as<NostrString>();
                JsonObject data = doc["result"];
                if (NostrString_equals(resultType, "get_balance")) {
                    out.result.balance = data["balance"];
                    return;
                }
            }
        }
    }
    if (NostrString_length(out.errorCode) == 0) {
        out.errorCode = "OTHER";
        out.errorMessage = "Invalid Event";
    }
}

void Nip47::parseResponse(SignedNostrEvent *response, Nip47Response<GetInfoResponse> &out) {
    out.errorCode = "";
    out.errorMessage = "";
    Utils::log("NWC: received get_info response");
    if (response->verify()) {
        NostrString content = this->nip04.decrypt(this->userPrivKey, this->servicePubKey, response->getContent());
        JsonDocument doc;
        Utils::jsonParse(&content, &doc);
        JsonObject error = doc["error"];
        if (!error.isNull()) {
            out.errorCode = error["code"].as<NostrString>();
            out.errorMessage = error["message"].as<NostrString>();
        } else {
            if (doc.containsKey("result") && doc.containsKey("result_type")) {
                NostrString resultType = doc["result_type"].as<NostrString>();
                JsonObject data = doc["result"];
                if (NostrString_equals(resultType, "get_info")) {
                    out.result.alias = data["alias"].as<NostrString>();
                    out.result.color = data["color"].as<NostrString>();
                    out.result.pubkey = data["pubkey"].as<NostrString>();
                    out.result.network = data["network"].as<NostrString>();
                    out.result.blockHeight = data["block_height"].as<unsigned long long>();
                    out.result.blockHash = data["block_hash"].as<NostrString>();
                    JsonArray methodsObj = data["methods"].as<JsonArray>();
                    for (JsonVariant v : methodsObj) {
                        out.result.methods.push_back(v.as<NostrString>());
                    }
                    JsonArray notificationsObj = data["notifications"].as<JsonArray>();
                    for (JsonVariant v : notificationsObj) {
                        out.result.notifications.push_back(v.as<NostrString>());
                    }
                    return;
                }
            }
        }
    }
    if (NostrString_length(out.errorCode) == 0) {
        out.errorCode = "OTHER";
        out.errorMessage = "Invalid Event";
    }
}

void Nip47::parseNWC(NostrString nwc, NWCData &data) {
    NostrString prefix = "nostr+walletconnect://";
    int prefixPos = NostrString_indexOf(nwc, prefix.c_str());
    if (prefixPos != -1) {
        NostrString withoutPrefix = NostrString_substring(nwc, prefixPos + NostrString_length(prefix));
        int queryPos = NostrString_indexOf(withoutPrefix, "?");
        if (queryPos != -1) {
            data.pubkey = NostrString_substring(withoutPrefix, 0, queryPos);
            NostrString queryParams = NostrString_substring(withoutPrefix, queryPos + 1);
            std::vector<NostrString> paramsXvals;
            NostrString_split(queryParams, '&', paramsXvals);
            for (NostrString paramXval : paramsXvals) {
                std::vector<NostrString> paramXvalSplit;
                NostrString_split(paramXval, '=', paramXvalSplit);
                if (paramXvalSplit.size() == 2) {
                    if (NostrString_equals(paramXvalSplit[0], "relay")) {
                        data.relay = NostrString_urlDecode(paramXvalSplit[1]);
                    } else if (NostrString_equals(paramXvalSplit[0], "secret")) {
                        data.secret = NostrString_urlDecode(paramXvalSplit[1]);
                    }
                }
            }
        }
    }
}
