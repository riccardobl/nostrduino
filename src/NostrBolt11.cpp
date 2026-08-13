#include "NostrBolt11.h"

#include "NostrUtils.h"
#include "crypto/UBitcoinBackend.h"

#include <cctype>
#include <cstring>
#include <limits>
#include <vector>

namespace {

const char *charset = "qpzry9x8gf2tvdw0s3jn54khce6mua7l";

uint32_t polymodStep(uint32_t value) {
    const uint8_t top = static_cast<uint8_t>(value >> 25);
    value = (value & 0x1ffffff) << 5;
    static const uint32_t generators[5] = {
        0x3b6a57b2, 0x26508e6d, 0x1ea119fa, 0x3d4233dd, 0x2a1462b3
    };
    for (size_t i = 0; i < 5; ++i) if ((top >> i) & 1) value ^= generators[i];
    return value;
}

bool convertBits(const uint8_t *input, size_t inputSize, unsigned int fromBits,
                 unsigned int toBits, bool pad, std::vector<uint8_t> &output) {
    uint32_t accumulator = 0;
    unsigned int bits = 0;
    const uint32_t maxValue = (1U << toBits) - 1U;
    const uint32_t maxAccumulator = (1U << (fromBits + toBits - 1U)) - 1U;
    output.clear();
    for (size_t i = 0; i < inputSize; ++i) {
        if ((input[i] >> fromBits) != 0) return false;
        accumulator = ((accumulator << fromBits) | input[i]) & maxAccumulator;
        bits += fromBits;
        while (bits >= toBits) {
            bits -= toBits;
            output.push_back(static_cast<uint8_t>((accumulator >> bits) & maxValue));
        }
    }
    if (pad) {
        if (bits != 0) output.push_back(static_cast<uint8_t>((accumulator << (toBits - bits)) & maxValue));
    } else if (bits >= fromBits || ((accumulator << (toBits - bits)) & maxValue) != 0) {
        return false;
    }
    return true;
}

bool parseAmount(const NostrString &hrp, nostr::MilliSats &amount, NostrString &currency) {
    if (hrp.length() < 4 || hrp[0] != 'l' || hrp[1] != 'n') return false;
    static const char *currencies[] = {"bcrt", "tbs", "bc", "tb"};
    size_t currencySize = 0;
    for (const char *candidate : currencies) {
        const size_t size = std::strlen(candidate);
        if (hrp.length() >= 2 + size &&
            NostrString_substring(hrp, 2, static_cast<int>(2 + size)) == candidate) {
            currency = candidate;
            currencySize = size;
            break;
        }
    }
    if (currencySize == 0) return false;
    const size_t amountStart = 2 + currencySize;
    if (amountStart >= hrp.length()) return false;
    size_t amountEnd = hrp.length();
    char multiplier = 0;
    const char last = hrp[hrp.length() - 1];
    if (last == 'm' || last == 'u' || last == 'n' || last == 'p') {
        multiplier = last;
        --amountEnd;
    }
    if (amountEnd <= amountStart || hrp[amountStart] == '0') return false;
    uint64_t value = 0;
    for (size_t i = amountStart; i < amountEnd; ++i) {
        if (hrp[i] < '0' || hrp[i] > '9') return false;
        const uint8_t digit = static_cast<uint8_t>(hrp[i] - '0');
        if (value > (std::numeric_limits<uint64_t>::max() - digit) / 10) return false;
        value = value * 10 + digit;
    }
    uint64_t millisats = 0;
    uint64_t factor = UINT64_C(100000000000);
    if (multiplier == 'm') factor = UINT64_C(100000000);
    if (multiplier == 'u') factor = UINT64_C(100000);
    if (multiplier == 'n') factor = UINT64_C(100);
    if (multiplier == 'p') {
        if (value % 10 != 0) return false;
        millisats = value / 10;
    } else {
        if (value > std::numeric_limits<uint64_t>::max() / factor) return false;
        millisats = value * factor;
    }
    if (millisats == 0 || millisats > static_cast<uint64_t>(INT64_MAX)) return false;
    amount = static_cast<nostr::MilliSats>(millisats);
    return true;
}

} // namespace

bool nostr::validateBolt11Invoice(const NostrString &invoice, MilliSats expectedAmount,
                                  const NostrString &expectedCurrency,
                                  unsigned long long nowSeconds) {
    static const size_t maxInvoiceBytes = 4096;
    if (!isRequiredMilliSatsValid(expectedAmount) || invoice.length() < 8 ||
        invoice.length() > maxInvoiceBytes) return false;
    bool hasLower = false;
    bool hasUpper = false;
    NostrString normalized = invoice;
    for (size_t i = 0; i < normalized.length(); ++i) {
        const unsigned char c = static_cast<unsigned char>(normalized[i]);
        hasLower = hasLower || std::islower(c);
        hasUpper = hasUpper || std::isupper(c);
        normalized[i] = static_cast<char>(std::tolower(c));
    }
    if (hasLower && hasUpper) return false;
    size_t separator = normalized.length();
    for (size_t i = 0; i < normalized.length(); ++i) if (normalized[i] == '1') separator = i;
    if (separator == normalized.length() || separator == 0 || separator + 7 > normalized.length()) return false;
    const NostrString hrp = NostrString_substring(normalized, 0, static_cast<int>(separator));
    MilliSats encodedAmount = 0;
    NostrString currency;
    if (!parseAmount(hrp, encodedAmount, currency) || encodedAmount != expectedAmount ||
        (NostrString_length(expectedCurrency) > 0 && currency != expectedCurrency)) return false;

    uint32_t checksum = 1;
    for (size_t i = 0; i < hrp.length(); ++i)
        checksum = polymodStep(checksum) ^ (static_cast<uint8_t>(hrp[i]) >> 5);
    checksum = polymodStep(checksum);
    for (size_t i = 0; i < hrp.length(); ++i)
        checksum = polymodStep(checksum) ^ (static_cast<uint8_t>(hrp[i]) & 31);
    std::vector<uint8_t> data;
    data.reserve(normalized.length() - separator - 1);
    for (size_t i = separator + 1; i < normalized.length(); ++i) {
        const char *position = std::strchr(charset, normalized[i]);
        if (position == nullptr) return false;
        const uint8_t value = static_cast<uint8_t>(position - charset);
        data.push_back(value);
        checksum = polymodStep(checksum) ^ value;
    }
    if (checksum != 1 || data.size() < 7 + 104 + 6) return false;
    data.resize(data.size() - 6);
    const size_t signatureStart = data.size() - 104;

    uint64_t timestamp = 0;
    for (size_t i = 0; i < 7; ++i) timestamp = (timestamp << 5) | data[i];
    uint64_t expiry = 3600;
    size_t paymentHashes = 0;
    size_t paymentSecrets = 0;
    size_t descriptions = 0;
    size_t descriptionHashes = 0;
    size_t expiries = 0;
    bool hasPayee = false;
    uint8_t payee[33] = {};
    size_t index = 7;
    while (index < signatureStart) {
        if (signatureStart - index < 3) return false;
        const char type = charset[data[index++]];
        const size_t fieldSize = (static_cast<size_t>(data[index]) << 5) | data[index + 1];
        index += 2;
        if (fieldSize > signatureStart - index) return false;
        if (type == 'p') {
            if (fieldSize != 52 || ++paymentHashes != 1) return false;
        } else if (type == 's') {
            if (fieldSize != 52 || ++paymentSecrets != 1) return false;
        } else if (type == 'd') {
            if (++descriptions != 1) return false;
        } else if (type == 'h') {
            if (fieldSize != 52 || ++descriptionHashes != 1) return false;
        } else if (type == 'x') {
            if (++expiries != 1 || (fieldSize > 0 && data[index] == 0)) return false;
            expiry = 0;
            if (fieldSize > 13) return false;
            for (size_t i = 0; i < fieldSize; ++i) {
                if (expiry > (std::numeric_limits<uint64_t>::max() >> 5)) return false;
                expiry = (expiry << 5) | data[index + i];
            }
        } else if (type == 'n') {
            if (hasPayee) return false;
            std::vector<uint8_t> decoded;
            if (!convertBits(data.data() + index, fieldSize, 5, 8, false, decoded) || decoded.size() != 33) return false;
            std::memcpy(payee, decoded.data(), sizeof(payee));
            hasPayee = true;
        }
        index += fieldSize;
    }
    if (index != signatureStart || paymentHashes != 1 || paymentSecrets != 1 ||
        descriptions + descriptionHashes != 1) return false;
    if (nowSeconds != 0 && (timestamp > UINT64_MAX - expiry || nowSeconds > timestamp + expiry)) return false;

    std::vector<uint8_t> signatureBytes;
    if (!convertBits(data.data() + signatureStart, 104, 5, 8, false, signatureBytes) ||
        signatureBytes.size() != 65 || signatureBytes[64] > 3) return false;
    std::vector<uint8_t> messageBytes;
    if (!convertBits(data.data(), signatureStart, 5, 8, true, messageBytes)) return false;
    std::vector<uint8_t> preimage;
    preimage.reserve(hrp.length() + messageBytes.size());
    const uint8_t *hrpBytes = reinterpret_cast<const uint8_t *>(NostrString_toChars(hrp));
    preimage.insert(preimage.end(), hrpBytes, hrpBytes + hrp.length());
    preimage.insert(preimage.end(), messageBytes.begin(), messageBytes.end());
    uint8_t digest[32] = {};
    if (Utils::sha256(preimage.data(), preimage.size(), digest) != sizeof(digest)) return false;
    const bool valid = crypto::UBitcoinBackend::verifyRecoverableEcdsa(
        digest, signatureBytes.data(), signatureBytes[64], hasPayee ? payee : nullptr);
    std::memset(digest, 0, sizeof(digest));
    std::memset(payee, 0, sizeof(payee));
    return valid;
}
