#include "NostrUrl.h"
#include <cctype>
#include <cstring>
#include <limits>

namespace {
bool validHost(const NostrString &host) {
    const long length = NostrString_length(host);
    if (length <= 0 || length > 253 || host[0] == '.' || host[0] == '-' ||
        host[length - 1] == '.' || host[length - 1] == '-') return false;
    char previous = 0;
    for (long i = 0; i < length; ++i) {
        const unsigned char c = static_cast<unsigned char>(host[i]);
        if (!(std::isalnum(c) || c == '.' || c == '-')) return false;
        if ((c == '.' && (previous == '.' || previous == '-')) ||
            (c == '-' && previous == '.')) return false;
        previous = static_cast<char>(c);
    }
    return true;
}

bool validPath(const NostrString &path) {
    const long length = NostrString_length(path);
    if (length <= 0 || path[0] != '/') return false;
    for (long i = 0; i < length; ++i) {
        const unsigned char c = static_cast<unsigned char>(path[i]);
        if (c <= 0x20 || c == 0x7f || c == '#') return false;
    }
    return true;
}

bool parsePort(const NostrString &value, uint16_t &port) {
    const long length = NostrString_length(value);
    if (length <= 0 || length > 5) return false;
    uint32_t parsed = 0;
    for (long i = 0; i < length; ++i) {
        const char c = value[i];
        if (c < '0' || c > '9') return false;
        parsed = parsed * 10 + static_cast<uint32_t>(c - '0');
    }
    if (parsed == 0 || parsed > std::numeric_limits<uint16_t>::max()) return false;
    port = static_cast<uint16_t>(parsed);
    return true;
}

bool parseUrl(const NostrString &url, const char *scheme, uint16_t defaultPort,
              bool secure, nostr::ParsedUrl &out) {
    out = nostr::ParsedUrl();
    const long urlLength = NostrString_length(url);
    const size_t schemeLength = std::strlen(scheme);
    if (urlLength <= 0 || static_cast<size_t>(urlLength) <= schemeLength ||
        !NostrString_startsWith(url, scheme)) return false;

    size_t authorityEnd = static_cast<size_t>(urlLength);
    for (size_t i = schemeLength; i < static_cast<size_t>(urlLength); ++i) {
        const unsigned char c = static_cast<unsigned char>(url[i]);
        if (c <= 0x20 || c == 0x7f || c == '#') return false;
        if (c == '/' || c == '?') {
            authorityEnd = i;
            break;
        }
    }
    if (authorityEnd == schemeLength) return false;

    const NostrString authority = NostrString_substring(url, static_cast<int>(schemeLength),
                                                        static_cast<int>(authorityEnd));
    const int colon = NostrString_indexOf(authority, ":");
    if (NostrString_indexOf(authority, "@") != -1 ||
        (colon != -1 && NostrString_indexOf(NostrString_substring(authority, colon + 1), ":") != -1)) return false;

    NostrString host = colon == -1 ? authority : NostrString_substring(authority, 0, colon);
    uint16_t port = defaultPort;
    if (!validHost(host) || (colon != -1 && !parsePort(NostrString_substring(authority, colon + 1), port))) return false;

    NostrString path = "/";
    if (authorityEnd < static_cast<size_t>(urlLength)) {
        if (url[authorityEnd] == '/') path = NostrString_substring(url, static_cast<int>(authorityEnd));
        else path += NostrString_substring(url, static_cast<int>(authorityEnd));
    }
    if (!validPath(path)) return false;

    out.host = host;
    out.path = path;
    out.port = port;
    out.secure = secure;
    return true;
}
}

bool nostr::parseWebSocketUrl(const NostrString &url, ParsedUrl &parsed) {
    if (parseUrl(url, "wss://", 443, true, parsed)) return true;
    return parseUrl(url, "ws://", 80, false, parsed);
}

bool nostr::parseHttpsUrl(const NostrString &url, ParsedUrl &parsed) {
    return parseUrl(url, "https://", 443, true, parsed);
}
