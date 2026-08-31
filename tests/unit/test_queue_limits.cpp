#include "test_unit.h"

#include "nwc_test_support.h"

namespace unit_test {

bool testQueueLimits() {
    using namespace nwc_mock;

    nostr::NostrLimits pendingLimits;
    pendingLimits.maxNwcPendingRequests = 2;
    Fixture pendingFixture(pendingLimits);
    int queueFullErrors = 0;
    auto queueError = [&](String code, String) {
        if (code == "QUEUE_FULL") ++queueFullErrors;
    };
    pendingFixture.client.getBalance(nullptr, queueError);
    pendingFixture.client.getInfo(nullptr, queueError);
    pendingFixture.client.getBalance(nullptr, queueError);
    CHECK(queueFullErrors == 1);
    CHECK(pendingFixture.negotiate());
    CHECK(pendingFixture.requestCount() == 2);
    pendingFixture.client.getInfo(nullptr, queueError);
    CHECK(queueFullErrors == 2);
    CHECK(pendingFixture.requestCount() == 2);

    Fixture timeoutFixture;
    int timeoutErrors = 0;
    timeoutFixture.client.getBalance(nullptr, [&](String code, String message) {
        if (code == "OTHER" && message == "timeout") ++timeoutErrors;
    });
    timeoutFixture.loop();
    setUnixTime(1700000000ULL + NWC_DEFAULT_TIMEOUT + 1ULL);
    timeoutFixture.loop();
    CHECK(timeoutErrors == 1);
    timeoutFixture.loop();
    CHECK(timeoutErrors == 1);
    CHECK(!timeoutFixture.client.isReady());
    CHECK(timeoutFixture.queueInfoEvent("nip44_v2"));
    timeoutFixture.loop();
    CHECK(!timeoutFixture.client.isReady());
    CHECK(timeoutFixture.requestCount() == 0);

    Fixture closeFixture;
    int closeErrors = 0;
    closeFixture.client.getBalance(nullptr, [&](String, String) { ++closeErrors; });
    closeFixture.client.close();
    CHECK(closeFixture.transport.connection == nullptr);
    CHECK(closeErrors == 0);

    nostr::NostrLimits relayLimits;
    relayLimits.maxRelayQueuedMessages = 1;
    Fixture relayFixture(relayLimits);
    int relayQueueErrors = 0;
    relayFixture.client.getBalance(nullptr, [&](String code, String) {
        if (code == "QUEUE_FULL") ++relayQueueErrors;
        relayFixture.client.close();
    });
    CHECK(relayFixture.queueInfoEvent("nip44_v2"));
    CHECK(relayFixture.queueInfoEose());
    relayFixture.loop();
    CHECK(relayQueueErrors == 1);
    CHECK(relayFixture.transport.connection == nullptr);
    CHECK(relayFixture.requestCount() == 0);

    return true;
}

} // namespace unit_test
