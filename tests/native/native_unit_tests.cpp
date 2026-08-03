#include <cstdint>
#include <iostream>
#include <limits>
#include <string>
#include <type_traits>

#include "NostrAmount.h"
#include "NostrNumeric.h"
#include "NostrString.h"

namespace {

bool check(bool condition, const char *expression, int line) {
    if (condition) return true;
    std::cerr << "NATIVE_ASSERT " << __FILE__ << ':' << line << ' ' << expression << '\n';
    return false;
}

#define NATIVE_CHECK(condition) \
    do { \
        if (!check((condition), #condition, __LINE__)) return false; \
    } while (false)

bool testAmounts() {
    static_assert(sizeof(nostr::MilliSats) == 8, "MilliSats must remain 64-bit");
    static_assert(std::is_signed<nostr::MilliSats>::value, "MilliSats must remain signed");
    static_assert(std::numeric_limits<nostr::MilliSats>::max() >= INT64_MAX,
                  "MilliSats must represent INT64_MAX");
    static_assert(nostr::MILLISATS_UNSPECIFIED == INT64_C(-1),
                  "the omission sentinel changed");
    static_assert(nostr::MAX_MILLISATS == INT64_C(100000000000000),
                  "the default amount guard changed");

    const struct {
        nostr::MilliSats value;
        const char *decimal;
    } cases[] = {
        {INT64_MIN, "-9223372036854775808"},
        {INT64_C(-1), "-1"},
        {INT64_C(0), "0"},
        {INT64_C(4294967295), "4294967295"},
        {INT64_C(4294967296), "4294967296"},
        {INT64_C(5000000000), "5000000000"},
        {INT64_C(100000000000), "100000000000"},
        {INT64_MAX, "9223372036854775807"},
    };
    for (const auto &testCase : cases) {
        NATIVE_CHECK(NostrString_fromMilliSats(testCase.value) == testCase.decimal);
    }

    NATIVE_CHECK(nostr::isOptionalMilliSatsValid(nostr::MILLISATS_UNSPECIFIED));
    NATIVE_CHECK(!nostr::isOptionalMilliSatsValid(INT64_C(-2)));
    NATIVE_CHECK(!nostr::isRequiredMilliSatsValid(INT64_C(0)));
    NATIVE_CHECK(nostr::isRequiredMilliSatsValid(nostr::MAX_MILLISATS));
#if NOSTRDUINO_ENABLE_AMOUNT_GUARDS
    NATIVE_CHECK(!nostr::isRequiredMilliSatsValid(nostr::MAX_MILLISATS + 1));
#else
    NATIVE_CHECK(nostr::isRequiredMilliSatsValid(nostr::MAX_MILLISATS + 1));
    NATIVE_CHECK(nostr::isRequiredMilliSatsValid(INT64_MAX));
#endif
    return true;
}

bool testNumericParsing() {
    int64_t integer = 77;
    NATIVE_CHECK(nostr::detail::parseIntegerStrict("0", integer) && integer == 0);
    NATIVE_CHECK(nostr::detail::parseIntegerStrict("-9223372036854775808", integer) &&
                 integer == INT64_MIN);
    NATIVE_CHECK(nostr::detail::parseIntegerStrict("9223372036854775807", integer) &&
                 integer == INT64_MAX);
    NATIVE_CHECK(!nostr::detail::parseIntegerStrict("", integer));
    NATIVE_CHECK(!nostr::detail::parseIntegerStrict(" 1", integer));
    NATIVE_CHECK(!nostr::detail::parseIntegerStrict("1 ", integer));
    NATIVE_CHECK(!nostr::detail::parseIntegerStrict("1x", integer));
    NATIVE_CHECK(!nostr::detail::parseIntegerStrict("1.0", integer));
    NATIVE_CHECK(!nostr::detail::parseIntegerStrict("9223372036854775808", integer));
    NATIVE_CHECK(!nostr::detail::parseIntegerStrict("-9223372036854775809", integer));

    double floating = 0;
    NATIVE_CHECK(nostr::detail::parseFloatStrict("1.25", floating) && floating == 1.25);
    NATIVE_CHECK(!nostr::detail::parseFloatStrict("", floating));
    NATIVE_CHECK(!nostr::detail::parseFloatStrict(" 1.25", floating));
    NATIVE_CHECK(!nostr::detail::parseFloatStrict("1.25x", floating));
    NATIVE_CHECK(!nostr::detail::parseFloatStrict("nan", floating));
    NATIVE_CHECK(!nostr::detail::parseFloatStrict("inf", floating));
    NATIVE_CHECK(!nostr::detail::parseFloatStrict("1e999", floating));
    return true;
}

} // namespace

int main() {
    const bool amountsPassed = testAmounts();
    const bool numericPassed = testNumericParsing();
    std::cout << "NATIVE_TEST amounts " << (amountsPassed ? "PASS" : "FAIL") << '\n';
    std::cout << "NATIVE_TEST numeric " << (numericPassed ? "PASS" : "FAIL") << '\n';
    const bool allPassed = amountsPassed && numericPassed;
    std::cout << (allPassed ? "NATIVE_TESTS PASSED" : "NATIVE_TESTS FAILED") << '\n';
    return allPassed ? 0 : 1;
}
