#include "test_unit.h"

#include "nwc_test_support.h"

namespace unit_test {

bool testNwcNegotiation() {
    using namespace nwc_mock;

    {
        Fixture fixture;
        fixture.client.getBalance();
        CHECK(!fixture.client.isReady());
        fixture.loop();
        CHECK(fixture.requestCount() == 0);
        CHECK(fixture.negotiate("nip04 nip44_v2"));
        CHECK(fixture.client.getEncryption() == nostr::Nip47Encryption::Nip44V2);
        CHECK(fixture.requestCount() == 1);
        PublishedRequest request;
        CHECK(fixture.requestAt(0, request));
        CHECK(request.encryption == "nip44_v2");
        JsonDocument plaintext;
        CHECK(fixture.decryptRequest(0, plaintext));
        CHECK(plaintext["method"].as<String>() == "get_balance");
    }

    {
        Fixture fixture;
        fixture.client.getBalance();
        CHECK(fixture.negotiate("nip04"));
        CHECK(fixture.client.getEncryption() == nostr::Nip47Encryption::Nip04);
        PublishedRequest request;
        CHECK(fixture.requestAt(0, request));
        CHECK(request.encryption == "nip04");
    }

    {
        Fixture fixture;
        fixture.client.getBalance();
        CHECK(fixture.negotiate("", false));
        CHECK(fixture.client.getEncryption() == nostr::Nip47Encryption::Nip04);
        PublishedRequest request;
        CHECK(fixture.requestAt(0, request));
        CHECK(request.encryption == "");
    }

    {
        Fixture fixture;
        String firstError;
        String secondError;
        fixture.client.getBalance(nullptr, [&](String code, String) { firstError = code; });
        CHECK(fixture.queueInfoEvent("future_mode"));
        fixture.loop();
        CHECK(!fixture.client.isReady());
        CHECK(firstError == "UNSUPPORTED_ENCRYPTION");
        fixture.client.getInfo(nullptr, [&](String code, String) { secondError = code; });
        CHECK(secondError == "UNSUPPORTED_ENCRYPTION");
        CHECK(fixture.requestCount() == 0);
    }

    {
        Fixture fixture;
        String errorCode;
        String errorMessage;
        fixture.client.getBalance(nullptr, [&](String code, String message) {
            errorCode = code;
            errorMessage = message;
        });
        fixture.loop();
        const String infoSubId = fixture.subscriptionIdForKind(13194);
        CHECK(infoSubId.length() == 64);
        fixture.transport.connection->queue(String("[\"EOSE\",\"") + infoSubId + "\"]");
        fixture.loop();
        CHECK(!fixture.client.isReady());
        CHECK(errorCode == "OTHER");
        CHECK(errorMessage.indexOf("not found") >= 0);
        CHECK(fixture.requestCount() == 0);
    }

    {
        Fixture fixture;
        String errorMessage;
        fixture.client.getBalance(nullptr, [&](String, String message) { errorMessage = message; });
        fixture.loop();
        const String infoSubId = fixture.subscriptionIdForKind(13194);
        CHECK(infoSubId.length() == 64);
        fixture.transport.connection->queue(String("[\"CLOSED\",\"") + infoSubId + "\",\"blocked\"]");
        fixture.loop();
        CHECK(errorMessage.indexOf("blocked") >= 0);
        CHECK(fixture.requestCount() == 0);
    }

    {
        Fixture fixture;
        String errorCode;
        fixture.client.getBalance(nullptr, [&](String code, String) { errorCode = code; });
        CHECK(fixture.queueInfoEvent("nip44_v2", true, fixture.otherPrivateKey));
        fixture.loop();
        CHECK(!fixture.client.isReady());
        CHECK(errorCode == "");
        const String infoSubId = fixture.subscriptionIdForKind(13194);
        fixture.transport.connection->queue(String("[\"EOSE\",\"") + infoSubId + "\"]");
        fixture.loop();
        CHECK(errorCode == "OTHER");
        CHECK(fixture.requestCount() == 0);
    }

    return true;
}

} // namespace unit_test
