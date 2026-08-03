#include "test_unit.h"

#include "ArduinoJson.h"
#include "NostrEvent.h"
#include "NostrLimits.h"

namespace unit_test {

bool parseRelayFrame(const char *json, nostr::SignedNostrEvent &event,
                     const nostr::NostrLimits &limits = nostr::NostrLimits()) {
    JsonDocument document;
    if (deserializeJson(document, json)) return false;
    return nostr::SignedNostrEvent::parseRelayFrame(document.as<JsonArrayConst>(), event, limits);
}

bool testParsing() {
    nostr::NostrEventTags tags;
    tags.addTag("p", {"one"});
    tags.addTag("e", {"two"});
    tags.addTag("a", {"three"});
    CHECK(tags.removeTag(0));
    CHECK(tags.count() == 2);
    CHECK(tags.removeTag(1));
    CHECK(tags.count() == 1);
    tags.addTag("p", {"four"});
    tags.addTag("e", {"five"});
    CHECK(tags.removeTag(1));
    CHECK(tags.count() == 2);
    CHECK(!tags.removeTag(tags.count()));
    CHECK(!tags.removeTag(tags.count() + 10));
    CHECK(tags.count() == 2);
    nostr::NostrEventTags emptyTags;
    CHECK(!emptyTags.removeTag(0));
    CHECK(emptyTags.count() == 0);

    nostr::SignedNostrEvent event;
    String valid = "[\"EVENT\",\"sub\",{\"id\":\"";
    valid += repeatChar('a', 64);
    valid += "\",\"pubkey\":\"";
    valid += repeatChar('b', 64);
    valid += "\",\"created_at\":1,\"kind\":1,\"tags\":[[\"p\",\"";
    valid += repeatChar('c', 64);
    valid += "\"]],\"content\":\"hello\",\"sig\":\"";
    valid += repeatChar('d', 128);
    valid += "\"}])";
    CHECK(parseRelayFrame(valid.c_str(), event));
    CHECK(event.isValid());
    CHECK(event.getContent() == "hello");
    CHECK(event.getSubId() == "sub");

    CHECK(!parseRelayFrame("[\"NOTICE\",\"sub\",{}]", event));
    CHECK(!event.isValid());
    String wrongIdLength = replaceRange(valid, valid.indexOf(repeatChar('a', 64)), 64, "aa");
    CHECK(!parseRelayFrame(wrongIdLength.c_str(), event));
    const int tagValue = valid.indexOf(repeatChar('c', 64));
    String wrongTagType = replaceRange(valid, tagValue - 1, 66, "7");
    CHECK(!parseRelayFrame(wrongTagType.c_str(), event));
    String invalidHex = valid;
    invalidHex.setCharAt(valid.indexOf(repeatChar('a', 64)), 'g');
    CHECK(!parseRelayFrame(invalidHex.c_str(), event));
    String oversizedSub = replaceRange(valid, valid.indexOf("sub"), 3, repeatChar('s', 65));
    CHECK(!parseRelayFrame(oversizedSub.c_str(), event));
    String negativeTimestamp = replaceRange(valid, valid.indexOf("\"created_at\":1"), 14, "\"created_at\":-1");
    CHECK(!parseRelayFrame(negativeTimestamp.c_str(), event));
    String stringKind = replaceRange(valid, valid.indexOf("\"kind\":1"), 8, "\"kind\":\"1\"");
    CHECK(!parseRelayFrame(stringKind.c_str(), event));

    nostr::NostrLimits small;
    small.maxEventContentBytes = 4;
    CHECK(!parseRelayFrame(valid.c_str(), event, small));
    small.maxEventContentBytes = 5;
    CHECK(parseRelayFrame(valid.c_str(), event, small));
    small.maxTagCount = 0;
    CHECK(!parseRelayFrame(valid.c_str(), event, small));
    return true;
}

} // namespace unit_test
