
#ifndef _NOSTR_EVENT_H
#define _NOSTR_EVENT_H
#include <Arduino.h>

#include "ArduinoJson.h"
#include "NostrString.h"
#include "NostrUtils.h"
#include <NostrAES.h>
#include <functional>
#include <initializer_list>
#include <map>
#include <vector>
namespace nostr {
using EventSubmissionFailureCallback = std::function<void(const String &)>;

class SignedNostrEvent;

typedef struct s_NostrEventTag {
    NostrString key;
    std::vector<NostrString> value;
} NostrEventTag;

class NostrEventTags {
  public:
    static const std::vector<NostrString> emptyTags;
    NostrEventTags(){};

    /**
     * Get the number of tags
     */
    unsigned int count() { return tags.size(); }

    /**
     * Get all tags
     */
    const std::vector<NostrEventTag> &getTags() const;

    /**
     * Get the first occurrence of a tag with the given key
     */
    const std::vector<NostrString> &getTag(NostrString key) const;

    /**
     * Add a tag with a given key and set of values
     * NB. You can add multiple tags with the same key
     */
    unsigned int addTag(NostrString key, std::initializer_list<NostrString> values);

    /**
     * Add a tag with a given key and set of values
     * NB. You can add multiple tags with the same key
     */

    unsigned int addTag(NostrString key, std::vector<NostrString> values);

    /**
     * Remove a tag at a given index
     */
    void removeTag(unsigned int index);

    /**
     * Remove all occurrences of a tag with the given key
     */
    void removeTags(NostrString key);

    /**
     * Remove all tags
     */
    void clearTags();

    /**
     * Output to json
     */
    void toJson(JsonArray doc) const;

  protected:
    std::vector<NostrEventTag> tags;
};

/**
 * Base class for NostrEvent
 */
class NostrEvent {
  public:
    NostrEvent(){};

    virtual ~NostrEvent(){};
    unsigned long int getCreatedAt() { return this->created_at; }
    unsigned int getKind() { return this->kind; }
    NostrEventTags *getTags() { return &this->tags; }
    NostrString getContent() { return this->content; };

  protected:
    unsigned int kind;
    NostrEventTags tags;
    NostrString content;
    unsigned long int created_at;
    NostrEvent(unsigned int kind, NostrEventTags tags, NostrString content, unsigned long int created_at) : kind(kind), tags(tags), content(content), created_at(created_at){};
};

/**
 * An unsigned NostrEvent.
 * This is a support class used to create a new nostr event.
 * When the event is ready to be sent, it should be signed with the sign
 * method, the result is a SignedNostrEvent that can be sent to the relay.
 */
class UnsignedNostrEvent : public NostrEvent {
  public:
    UnsignedNostrEvent() : NostrEvent(){};
    UnsignedNostrEvent(unsigned int kind, NostrString content, unsigned long int created_at) : NostrEvent(kind, NostrEventTags(), content, created_at){};

    /**
     * Sign the event with a private key
     */
    SignedNostrEvent sign(NostrString privateKeyHex);

    void setCreatedAt(unsigned long int created_at) { this->created_at = created_at; };
    void setKind(unsigned int kind) { this->kind = kind; };

    void setContent(NostrString content) { this->content = content; };
};

/**
 * A signed NostrEvent as it is received by the client and sent to the
 * relay.
 */
class SignedNostrEvent : public NostrEvent {
    friend class UnsignedNostrEvent;
    friend class NostrPool;

  private:
    NostrString id;
    NostrString pubkey;
    NostrString signature;
    NostrString subid;
    bool stored;

  public:
    
    NostrString getId() { return this->id; }
    bool isStored() { return this->stored; }

    NostrString getPubkey() { return this->pubkey; };

    const NostrEventTags *getTags() const { return &this->tags; };

    NostrString getSignature() const { return this->signature; };
    NostrString getSubId() const { return this->subid; };
    void toSendableEvent(JsonArray doc) const;
    void toJson(JsonObject doc) const;
    bool verify() const;

  protected:
    SignedNostrEvent(JsonArray relayEvent);
    SignedNostrEvent(NostrString id, NostrString pubkey, unsigned long int created_at, unsigned int kind, NostrEventTags tags, NostrString content, NostrString signature)
        : NostrEvent(kind, tags, content, created_at), id(id), pubkey(pubkey), signature(signature), stored(false){};
};

} 
#endif