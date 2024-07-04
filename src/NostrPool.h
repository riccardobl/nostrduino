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
    using NostrEventStatusCallback =
        std::function<void(const NostrString &, bool, const NostrString& )>;
    class NostrSubscription {
       public:
        bool eose = false;
        NostrCloseCallback closeCallback = nullptr;
        NostrEOSECallback eoseCallback = nullptr;
        NostrEventCallback eventCallback = nullptr;
        NostrEventStatusCallback statusCallback = nullptr;
    };

    typedef struct s_EventStatusCallbackEntry {
        NostrEventStatusCallback statusCallback;
        long long timestampSeconds;
        NostrString eventId;
    } EventStatusCallbackEntry;

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
      
        NostrNoticeCallback noticeCallback = nullptr;

        long long subs = 0;
        std::map<NostrString, NostrSubscription> subscriptions;
        std::vector<NostrRelay> relays;
        void onEvent(NostrRelay *relay, WStype_t type, uint8_t *payload,
                     size_t length);
        int eventStatusTimeoutSeconds = 60*10;
        std::vector<EventStatusCallbackEntry>    eventStatusCallbackEntries;
       public:
        NostrPool(int eventStatusTimeoutSeconds=60*10) {
            this->eventStatusTimeoutSeconds = eventStatusTimeoutSeconds;
        };
        NostrString subscribeMany(
            std::initializer_list<NostrString> urls,
            std::initializer_list<
                std::map<NostrString, std::initializer_list<NostrString>>>
                filters,
            NostrEventCallback eventCallback = nullptr,
            NostrCloseCallback closeCallback = nullptr,
            NostrEOSECallback eoseCallback = nullptr
      
        );
        void closeSubscription(NostrString subId);
        NostrRelay *ensureRelay(NostrString url);

        void disconnectRelay(NostrString url);
        void close();
        void publish(std::initializer_list<NostrString> rs,
                     SignedNostrEvent *event,
                     NostrEventStatusCallback statusCallback = nullptr);
        void loop();
    };
}  // namespace nostr
#endif