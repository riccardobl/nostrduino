#include "NostrEncoding.h"
#include "NostrEvent.h"
#include <cassert>
#include <string>

namespace nostr {
bool Utils::decodeHexExact(const NostrString &input, uint8_t *output, size_t outputCapacity, size_t expectedBytes) {
    return ::nostr::decodeHexExact(input, output, outputCapacity, expectedBytes);
}
bool Utils::decodeHexExact(const NostrString &input, uint8_t *output, size_t expectedBytes) {
    return ::nostr::decodeHexExact(input, output, expectedBytes, expectedBytes);
}
}

static bool parse(const char *json, nostr::SignedNostrEvent &event,
                  const nostr::NostrLimits &limits = nostr::NostrLimits()) {
    JsonDocument document;
    if (deserializeJson(document, json)) return false;
    return nostr::SignedNostrEvent::parseRelayFrame(document.as<JsonArrayConst>(), event, limits);
}

int main() {
    nostr::SignedNostrEvent event;
    const std::string valid = std::string("[\"EVENT\",\"sub\",{\"id\":\"") + std::string(64, 'a') +
        "\",\"pubkey\":\"" + std::string(64, 'b') +
        "\",\"created_at\":1,\"kind\":1,\"tags\":[[\"p\",\"" + std::string(64, 'c') +
        "\"]],\"content\":\"hello\",\"sig\":\"" + std::string(128, 'd') + "\"}])";
    assert(parse(valid.c_str(), event));
    assert(event.isValid());
    assert(event.getContent() == "hello");
    assert(event.getSubId() == "sub");

    const char *wrongCommand = R"json(["NOTICE","sub",{}])json";
    assert(!parse(wrongCommand, event));
    assert(!event.isValid());
    std::string wrongIdLength = valid;
    wrongIdLength.replace(wrongIdLength.find(std::string(64, 'a')), 64, "aa");
    assert(!parse(wrongIdLength.c_str(), event));
    std::string wrongTagType = valid;
    const size_t tagValue = wrongTagType.find(std::string(64, 'c'));
    wrongTagType.replace(tagValue - 1, 66, "7");
    assert(!parse(wrongTagType.c_str(), event));

    std::string invalidHex = valid;
    invalidHex[invalidHex.find(std::string(64, 'a'))] = 'g';
    assert(!parse(invalidHex.c_str(), event));
    std::string oversizedSub = valid;
    oversizedSub.replace(oversizedSub.find("sub"), 3, std::string(65, 's'));
    assert(!parse(oversizedSub.c_str(), event));
    std::string negativeTimestamp = valid;
    negativeTimestamp.replace(negativeTimestamp.find("\"created_at\":1"), 14, "\"created_at\":-1");
    assert(!parse(negativeTimestamp.c_str(), event));
    std::string stringKind = valid;
    stringKind.replace(stringKind.find("\"kind\":1"), 8, "\"kind\":\"1\"");
    assert(!parse(stringKind.c_str(), event));

    nostr::NostrLimits small;
    small.maxEventContentBytes = 4;
    assert(!parse(valid.c_str(), event, small));
    small.maxEventContentBytes = 5;
    assert(parse(valid.c_str(), event, small));
    small.maxTagCount = 0;
    assert(!parse(valid.c_str(), event, small));
    return 0;
}
