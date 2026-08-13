#include "test_unit.h"

#include "NostrBolt11.h"

namespace unit_test {

bool testBolt11() {
    const String invoice =
        "lnbc2500u1pvjluezsp5zyg3zyg3zyg3zyg3zyg3zyg3zyg3zyg3zyg3zyg3zyg3zyg3zygs"
        "pp5qqqsyqcyq5rqwzqfqqqsyqcyq5rqwzqfqqqsyqcyq5rqwzqfqypqdq5xysxxatsyp3k"
        "7enxv4jsxqzpu9qrsgquk0rl77nj30yxdy8j9vdx85fkpmdla2087ne0xh8nhedh8w27kyke"
        "0lp53ut353s06fv3qfegext0eh0ymjpf39tuven09sam30g4vgpfna3rh";
    CHECK(nostr::validateBolt11Invoice(invoice, INT64_C(250000000), "bc", 0));
    CHECK(!nostr::validateBolt11Invoice(invoice, INT64_C(250000001), "bc", 0));
    CHECK(!nostr::validateBolt11Invoice(invoice, INT64_C(250000000), "tb", 0));
    CHECK(!nostr::validateBolt11Invoice(invoice, INT64_C(250000000), "bc", 1496314719ULL));

    String uppercase = invoice;
    uppercase.toUpperCase();
    CHECK(nostr::validateBolt11Invoice(uppercase, INT64_C(250000000), "bc", 0));
    String badChecksum = invoice;
    badChecksum.setCharAt(badChecksum.length() - 1, badChecksum.endsWith("q") ? 'p' : 'q');
    CHECK(!nostr::validateBolt11Invoice(badChecksum, INT64_C(250000000), "bc", 0));
    return true;
}

} // namespace unit_test
