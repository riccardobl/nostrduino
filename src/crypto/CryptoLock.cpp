#include "CryptoLock.h"

std::mutex &nostr::crypto::uBitcoinMutex() {
    static std::mutex mutex;
    return mutex;
}
