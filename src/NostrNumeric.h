#ifndef NOSTR_NUMERIC_H
#define NOSTR_NUMERIC_H

#include "NostrString.h"
#include <cerrno>
#include <cctype>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <limits>

namespace nostr {
namespace detail {

inline bool parseIntegerStrict(const NostrString &value, int64_t &out) {
    const char *text = NostrString_toChars(value);
    if (text == nullptr || *text == '\0' ||
        std::isspace(static_cast<unsigned char>(*text))) return false;
    errno = 0;
    char *end = nullptr;
    const long long parsed = std::strtoll(text, &end, 10);
    if (errno == ERANGE || end == nullptr || end == text || *end != '\0' ||
        parsed < static_cast<long long>(std::numeric_limits<int64_t>::min()) ||
        parsed > static_cast<long long>(std::numeric_limits<int64_t>::max())) return false;
    out = static_cast<int64_t>(parsed);
    return true;
}

inline bool parseFloatStrict(const NostrString &value, double &out) {
    const char *text = NostrString_toChars(value);
    if (text == nullptr || *text == '\0' ||
        std::isspace(static_cast<unsigned char>(*text))) return false;
    errno = 0;
    char *end = nullptr;
    const double parsed = std::strtod(text, &end);
    if (errno == ERANGE || end == nullptr || end == text || *end != '\0' ||
        !std::isfinite(parsed)) return false;
    out = parsed;
    return true;
}

} // namespace detail
} // namespace nostr

#endif
