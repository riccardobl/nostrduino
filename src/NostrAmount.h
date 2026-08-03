#ifndef NOSTR_AMOUNT_H
#define NOSTR_AMOUNT_H

#include <cstdint>
#include <limits>
#include <type_traits>

#ifndef NOSTRDUINO_ENABLE_AMOUNT_GUARDS
#define NOSTRDUINO_ENABLE_AMOUNT_GUARDS 1
#endif

#ifndef NOSTRDUINO_MAX_AMOUNT_MSATS
#define NOSTRDUINO_MAX_AMOUNT_MSATS INT64_C(100000000000000)
#endif

namespace nostr {

using MilliSats = int64_t;

static constexpr MilliSats MILLISATS_UNSPECIFIED = INT64_C(-1);
static constexpr MilliSats MAX_MILLISATS = NOSTRDUINO_MAX_AMOUNT_MSATS;

static_assert(sizeof(MilliSats) == 8, "MilliSats must be a signed 64-bit integer");
static_assert(std::is_signed<MilliSats>::value, "MilliSats must be signed");
static_assert(std::numeric_limits<MilliSats>::max() >= INT64_MAX,
              "MilliSats must represent INT64_MAX");
static_assert(MILLISATS_UNSPECIFIED < 0, "the unspecified amount sentinel must be negative");
static_assert(MAX_MILLISATS > 0, "the configured millisatoshi maximum must be positive");
static_assert(NOSTRDUINO_MAX_AMOUNT_MSATS <= INT64_MAX,
              "the configured millisatoshi maximum must fit in MilliSats");

inline bool amountWithinConfiguredMaximum(MilliSats amount) {
#if NOSTRDUINO_ENABLE_AMOUNT_GUARDS
    return amount <= MAX_MILLISATS;
#else
    (void)amount;
    return true;
#endif
}

inline bool isRequiredMilliSatsValid(MilliSats amount) {
    return amount > 0 && amountWithinConfiguredMaximum(amount);
}

inline bool isOptionalMilliSatsValid(MilliSats amount) {
    return amount >= MILLISATS_UNSPECIFIED &&
           (amount <= 0 || amountWithinConfiguredMaximum(amount));
}

} // namespace nostr

#endif
