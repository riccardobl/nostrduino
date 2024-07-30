#ifndef _NOSTR_POOL_H
#define _NOSTR_POOL_H
#include <Arduino.h>
#include <NostrTransport.h>
#include <functional>
#include <initializer_list>
#include <map>
#include <vector>
#include "NostrString.h"
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
        Connection *getConnection() const { return conn; }
        NostrString getUrl() const{
            return url;
        }
       protected:
         Connection *conn;
         NostrString url;
         std::vector<NostrString> messageQueue;
         void processQueue();
    };

    /**
     * The main class for interacting with the Nostr network.
     * Handles subscriptions, relays, and event publishing.
     */
    class NostrPool {
      
       public:
         /**
          * Create a new NostrPool
          * @param transport A transport object, you can get one for the esp32 platform using nostr::esp32::ESP32Platform::getTransport() , for other platforms you need to implement the Transport
          * interface
          * @param eventStatusTimeoutSeconds The number of seconds to wait for an event status before timing out (optional)
          */
         NostrPool(Transport *transport, int eventStatusTimeoutSeconds = 60 * 10) {
             this->transport = transport;
             this->eventStatusTimeoutSeconds = eventStatusTimeoutSeconds;
        };

        /**
         * Subscribe to events from one or more relays
         * @param urls The relays to subscribe to
         * @param filters A list of filters for each relay
         * @param eventCallback A callback to be called when an event is received (optional)
         * @param closeCallback A callback to be called when the subscription is closed (optional)
         * @param eoseCallback A callback to be called when the stored events are exhausted (optional) 
         * @return The subscription ID
         */
        NostrString subscribeMany(
            std::initializer_list<NostrString> urls,
            std::initializer_list<
                std::map<NostrString, std::initializer_list<NostrString>>>
                filters,
            NostrEventCallback eventCallback = nullptr,
            NostrCloseCallback closeCallback = nullptr,
            NostrEOSECallback eoseCallback = nullptr
      
        );

        /**
         * Subscribe to events from one or more relays
         * @param urls The relays to subscribe to
         * @param filters A json array of filters for each relay
         * @param eventCallback A callback to be called when an event is received (optional)
         * @param closeCallback A callback to be called when the subscription is closed (optional)
         * @param eoseCallback A callback to be called when the stored events are exhausted (optional)
         * @return The subscription ID
         */
        NostrString subscribeMany(std::initializer_list<NostrString> urls, JsonArray filters,
                                  NostrEventCallback eventCallback = nullptr, NostrCloseCallback closeCallback = nullptr, NostrEOSECallback eoseCallback = nullptr

        );
        /**
         * Publish a signed event to one or more relays. The pool will automatically start listening to the specified relays after publishing.
         * @param rs The relays to publish to
         * @param event The event to publish
         * @param statusCallback A callback to be called when the event status is known (optional)
         */
        void publish(std::initializer_list<NostrString> rs,
                     SignedNostrEvent *event,
                     NostrEventStatusCallback statusCallback = nullptr);

        /**
         * Tick the NostrPool. This should be called in the main loop of your sketch
         */
        void loop();

        /**
         * Close the subscription with the given ID
         * @param subId The subscription ID
         */
        void closeSubscription(NostrString subId);

        /**
         * Ensure the connection to the specified relay is open
         * @param url The relay URL
         * @return The relay object
         */
        NostrRelay *ensureRelay(NostrString url);

        /**
         * Get all relays that the pool is connected to
         * @return A list of relay URLs
         * @deprecated Use getConnectedRelays() instead  
         */
        std::vector<NostrString> getRelays();

        /**
         * Disconnect from the specified relay
         * @param url The relay URL
         */
        void disconnectRelay(NostrString url);

        /**
         * Close the pool and disconnect from every relay
         */
        void close();

  
        /**
         * Get all relays that the pool is connected to
         * @return A list of relays
         */
        std::vector<NostrRelay *> *getConnectedRelays();

      private:
        NostrNoticeCallback noticeCallback = nullptr;
        long long subs = 0;
        std::map<NostrString, NostrSubscription> subscriptions;
        std::vector<NostrRelay *> relays;
        void onEvent(NostrRelay *relay, NostrString message);
        int eventStatusTimeoutSeconds = 60 * 10;
        std::vector<EventStatusCallbackEntry> eventStatusCallbackEntries;
        Transport *transport;
    };
}  // namespace nostr
#endif