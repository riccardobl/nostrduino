#include "test_unit.h"

#include <vector>

#include "ArduinoJson.h"
#include "Nip04.h"
#include "Nip44.h"
#include "Nip47.h"
#include "NostrEvent.h"
#include "NostrUtils.h"

namespace unit_test {

bool testEventsAndWalletApi() {
    String userPrivateKey = "0000000000000000000000000000000000000000000000000000000000000001";
    String servicePrivateKey = "0000000000000000000000000000000000000000000000000000000000000002";
    String userPublicKey = nostr::Utils::getPublicKey(userPrivateKey);
    String servicePublicKey = nostr::Utils::getPublicKey(servicePrivateKey);
    CHECK(userPublicKey == "79be667ef9dcbbac55a06295ce870b07029bfcdb2dce28d959f2815b16f81798");
    CHECK(servicePublicKey == "c6047f9441ed7d6d3045406e95c07cd85c778e4b8cef3ca7abac09b95c709ee5");

    nostr::NostrEventTags tags;
    CHECK(tags.count() == 0);
    CHECK(tags.addTag("p", {servicePublicKey}) == 1);
    CHECK(tags.addTag("e", std::vector<String>{"event-ref"}) == 2);
    CHECK(tags.getTag("p").size() == 1 && tags.getTag("p")[0] == servicePublicKey);
    CHECK(tags.getTag("missing").empty());
    JsonDocument tagsDocument;
    JsonArray tagsJson = tagsDocument.to<JsonArray>();
    tags.toJson(tagsJson);
    CHECK(tagsJson.size() == 2 && tagsJson[0][0].as<String>() == "p");
    tags.removeTags("p");
    CHECK(tags.count() == 1);
    tags.removeTag(0);
    CHECK(tags.count() == 0);
    tags.clearTags();

    nostr::UnsignedNostrEvent unsignedEvent(1, "event payload", 1700000000);
    unsignedEvent.getTags()->addTag("p", {servicePublicKey});
    nostr::SignedNostrEvent signedEvent = unsignedEvent.sign(userPrivateKey);
    CHECK(signedEvent.isValid());
    CHECK(signedEvent.getKind() == 1 && signedEvent.getCreatedAt() == 1700000000);
    CHECK(signedEvent.getPubkey() == userPublicKey && signedEvent.getContent() == "event payload");
    CHECK(signedEvent.getTags()->getTag("p").size() == 1);
    CHECK(signedEvent.verify());
    CHECK(signedEvent.verify());
    JsonDocument eventDocument;
    JsonObject eventJson = eventDocument.to<JsonObject>();
    signedEvent.toJson(eventJson);
    CHECK(eventJson["id"].as<String>() == signedEvent.getId());
    JsonDocument sendDocument;
    JsonArray sendArray = sendDocument.to<JsonArray>();
    signedEvent.toSendableEvent(sendArray);
    CHECK(sendArray.size() == 2 && sendArray[0].as<String>() == "EVENT");

    nostr::Utils::setUnixTimeSecondsProvider([]() -> unsigned long long { return 1700000000ULL; });
    nostr::Nip04 nip04;
    nostr::Nip47 nip47(nip04, servicePublicKey, userPrivateKey);
    auto checkMethod = [&](nostr::SignedNostrEvent event, const char *method) {
        CHECK(event.isValid());
        CHECK(event.getKind() == 23194);
        CHECK(event.getTags()->getTag("p").size() == 1);
        CHECK(event.getTags()->getTag("encryption").empty());
        const String decrypted = nip04.decrypt(userPrivateKey, servicePublicKey, event.getContent());
        CHECK(decrypted.indexOf(String("\"method\":\"") + method + "\"") >= 0);
        CHECK(event.verify());
        return true;
    };
    CHECK(checkMethod(nip47.payInvoice("lnbc-test", 21), "pay_invoice"));
    CHECK(checkMethod(nip47.multiPayInvoice({{"lnbc-a", 1}, {"lnbc-b", 2}}), "multi_pay_invoice"));
    CHECK(checkMethod(nip47.payKeySend(servicePublicKey, 1000, "preimage", {{6969, "value"}}), "pay_keysend"));
    CHECK(checkMethod(nip47.multiPayKeySend({{servicePublicKey, 1000, "", {}}}), "multi_pay_keysend"));
    CHECK(checkMethod(nip47.makeInvoice(1000, "description", "hash", 60), "make_invoice"));
    CHECK(checkMethod(nip47.lookUpPaymentHash("payment-hash"), "lookup_invoice"));
    CHECK(checkMethod(nip47.lookUpInvoice("lnbc-lookup"), "lookup_invoice"));
    CHECK(checkMethod(nip47.listTransactions(1, 2, 3, 4, true, "incoming"), "list_transactions"));
    nostr::SignedNostrEvent balanceRequest = nip47.getBalance();
    CHECK(checkMethod(balanceRequest, "get_balance"));
    CHECK(checkMethod(nip47.getInfo(), "get_info"));

    nostr::Nip47 nip47Nip44(nip04, servicePublicKey, userPrivateKey);
    nip47Nip44.setEncryption(nostr::Nip47Encryption::Nip44V2);
    nostr::SignedNostrEvent nip44Request = nip47Nip44.getBalance();
    CHECK(nip44Request.getTags()->getTag("encryption").size() == 1);
    CHECK(nip44Request.getTags()->getTag("encryption")[0] == "nip44_v2");
    nostr::Nip44 nip44;
    String nip44Content = nip44Request.getContent();
    const String nip44Plaintext = nip44.decrypt(servicePrivateKey, userPublicKey, nip44Content);
    CHECK(nip44Plaintext.indexOf("\"method\":\"get_balance\"") >= 0);

    nostr::Nip47 nip47ExplicitNip04(nip04, servicePublicKey, userPrivateKey);
    nip47ExplicitNip04.setEncryption(nostr::Nip47Encryption::Nip04);
    nostr::SignedNostrEvent nip04Request = nip47ExplicitNip04.getBalance();
    CHECK(nip04Request.getTags()->getTag("encryption").size() == 1);
    CHECK(nip04Request.getTags()->getTag("encryption")[0] == "nip04");
    const String nip04Plaintext = nip04.decrypt(userPrivateKey, servicePublicKey, nip04Request.getContent());
    CHECK(nip04Plaintext.indexOf("\"method\":\"get_balance\"") >= 0);

    nostr::NWCData nwc;
    CHECK(nostr::Nip47::parseNWC("nostr+walletconnect://" + servicePublicKey +
                                 "?relay=wss%3A%2F%2Frelay.example%2Fv1&secret=" + userPrivateKey, nwc));
    CHECK(nwc.pubkey == servicePublicKey && nwc.secret == userPrivateKey);
    CHECK(nwc.relay == "wss://relay.example/v1");
    CHECK(nwc.relays.size() == 1 && nwc.relays[0] == nwc.relay);

    JsonDocument responseDocument;
    responseDocument["result_type"] = "get_balance";
    responseDocument["result"]["balance"] = 4242ULL;
    String responseContent;
    nostr::Utils::jsonStringify(responseDocument.as<JsonVariantConst>(), &responseContent);
    String encryptedResponse = nip04.encrypt(servicePrivateKey, userPublicKey, responseContent);
    nostr::UnsignedNostrEvent unsignedResponse(23195, encryptedResponse, 1700000000);
    unsignedResponse.getTags()->addTag("p", {userPublicKey});
    unsignedResponse.getTags()->addTag("e", {balanceRequest.getId()});
    nostr::SignedNostrEvent response = unsignedResponse.sign(servicePrivateKey);
    nostr::Nip47Response<nostr::GetBalanceResponse> parsedResponse;
    nip47.parseResponse(&response, parsedResponse);
    CHECK(parsedResponse.errorCode == "" && parsedResponse.result.balance == 4242ULL);

    String nip44EncryptedResponse = nip44.encrypt(servicePrivateKey, userPublicKey, responseContent);
    nostr::UnsignedNostrEvent unsignedNip44Response(23195, nip44EncryptedResponse, 1700000000);
    unsignedNip44Response.getTags()->addTag("p", {userPublicKey});
    unsignedNip44Response.getTags()->addTag("e", {nip44Request.getId()});
    nostr::SignedNostrEvent nip44Response = unsignedNip44Response.sign(servicePrivateKey);
    nostr::Nip47Response<nostr::GetBalanceResponse> parsedNip44Response;
    nip47Nip44.parseResponse(&nip44Response, parsedNip44Response);
    CHECK(parsedNip44Response.errorCode == "" && parsedNip44Response.result.balance == 4242ULL);
    return true;
}

} // namespace unit_test
