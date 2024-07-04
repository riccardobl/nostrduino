#ifndef NOSTRPOOL_H
#define NOSTRPOOL_H
 
#include <WebSocketsClient.h>
#include <iostream>
#include <map>
#include <memory>
#include <vector>
#include "ArduinoJson.h"
#include "NostrEvent.h"
#include "NostrString.h"

namespace nostr {
    class NostrRelay;

    using NostrCloseCallback =
        std::function<void(const NostrString &, const NostrString &)>;
    using NostrNoticeCallback =
        std::function<void(NostrRelay *, const NostrString &)>;
    using NostrEOSECallback = std::function<void(const NostrString &)>;
    using NostrEventCallback =
        std::function<void(const NostrString &, SignedNostrEvent *event)>;

    class NostrSubscription {
       public:
        bool eose = false;
        std::vector<SignedNostrEvent> events;
        NostrCloseCallback closeCallback = nullptr;
        NostrEOSECallback eoseCallback = nullptr;
        NostrEventCallback eventCallback = nullptr;
    };

    class NostrRelay {
        friend class NostrPool;

       public:
        /**
         * Send a message to the relay
         */
        void send(NostrString message);
        NostrRelay(WebSocketsClient ws, NostrString url) : ws(ws), url(url){};

       protected:
        std::vector<NostrString> messageQueue;
        WebSocketsClient ws;
        NostrString url;
        void processQueue();
    };

    class NostrPool {
       private:
        std::map<NostrString, std::weak_ptr<SentNostrEventStatus>>
            sentEventsStatus;

        NostrNoticeCallback noticeCallback = nullptr;

        long subs = 0;
        int maxPendingEvents;
        std::map<NostrString, NostrSubscription> subscriptions;
        std::vector<NostrRelay> relays;
        void onEvent(NostrRelay *relay, WStype_t type, uint8_t *payload,
                     size_t length);
        void enqueueEvent(NostrString subId, SignedNostrEvent &event);

       public:
        NostrPool(int maxPendingEvents) : maxPendingEvents(maxPendingEvents){};
        NostrPool() : maxPendingEvents(-1){};
        std::vector<SignedNostrEvent> *getEvents(NostrString subId);
        void clearEvents();
        NostrString subscribeMany(
            std::initializer_list<NostrString> urls,
            std::initializer_list<
                std::map<NostrString, std::initializer_list<NostrString>>>
                filters,
            NostrEventCallback eventCallback = nullptr,
            NostrCloseCallback closeCallback = nullptr,
            NostrEOSECallback eoseCallback = nullptr);
        void closeSubscription(NostrString subId);
        NostrRelay *ensureRelay(NostrString url);

        void disconnectRelay(NostrString url);
        void close();
        std::shared_ptr<SentNostrEventStatus> publish(
            std::initializer_list<NostrString> rs, SignedNostrEvent *event);
        void loop();
        void commit();
    };
}  // namespace nostr
#endif