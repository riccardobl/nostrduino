#ifndef _NOSTR_POOL_H
#define _NOSTR_POOL_H

#include <Transport.h>
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
        NostrRelay(Connection *conn, NostrString url) : conn(conn), url(url){};

       protected:
        std::vector<NostrString> messageQueue;
        Connection *conn;
        NostrString url;
        void processQueue();
    };

    class NostrPool {
       private:
      
        NostrNoticeCallback noticeCallback = nullptr;

        long long subs = 0;
        std::map<NostrString, NostrSubscription> subscriptions;
        std::vector<NostrRelay*> relays;
        void onEvent(NostrRelay *relay, NostrString message);
        int eventStatusTimeoutSeconds = 60*10;
        std::vector<EventStatusCallbackEntry>    eventStatusCallbackEntries;
        Transport* transport;
       public:
        NostrPool(Transport* transport,int eventStatusTimeoutSeconds=60*10) {
            this->transport = transport;
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
        std::vector<NostrString> getRelays();

        void disconnectRelay(NostrString url);
        void close();
        void publish(std::initializer_list<NostrString> rs,
                     SignedNostrEvent *event,
                     NostrEventStatusCallback statusCallback = nullptr);
        void loop();
    };
}  // namespace nostr
#endif