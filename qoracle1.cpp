/*
BANKON PYTHAI - Quantum Oracle Validator Smart Contract
Production, audit-optimized C++ for Qubic

This contract enables quantum-safe, threshold-validated oracle feeds.
Features: Multi-sig, anti-replay, deterministic serialization, strict price/time bounds.
All math is fixed-point (uint64_t), no floats, no protocol profit.

LICENCE BLOCK:
Permission is hereby granted, perpetual, worldwide, non-exclusive, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The Software cannot be used in any form or in any substantial portions for development, maintenance and for any other purposes, in the military sphere and in relation to military products, including, but not limited to:
a. any kind of armored force vehicles, missile weapons, warships, artillery weapons, air military vehicles (including military aircrafts, combat helicopters, military drones aircrafts), air defense systems, rifle armaments, small arms, firearms and side arms, melee weapons, chemical weapons, weapons of mass destruction;
b. any special software for development technical documentation for military purposes;
c. any special equipment for tests of prototypes of any subjects with military purpose of use;
d. any means of protection for conduction of acts of a military nature;
e. any software or hardware for determining strategies, reconnaissance, troop positioning, conducting military actions, conducting special operations;
f. any dual-use products with possibility to use the product in military purposes;
g. any other products, software or services connected to military activities;
h. any auxiliary means related to abovementioned spheres and products.

The Software cannot be used as described herein in any connection to the military activities. A person, a company, or any other entity, which wants to use the Software, shall take all reasonable actions to make sure that the purpose of use of the Software cannot be possibly connected to military purposes.

The Software cannot be used by a person, a company, or any other entity, activities of which are connected to military sphere in any means. If a person, a company, or any other entity, during the period of time for the usage of Software, would engage in activities, connected to military purposes, such person, company, or any other entity shall immediately stop the usage of Software and any its modifications or alterations.

Abovementioned restrictions should apply to all modification, alteration, merge, and to other actions, related to the Software, regardless of how the Software was changed due to the abovementioned actions.

The above copyright notice and this permission notice shall be included in all copies or substantial portions, modifications and alterations of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

// ====== Includes ======
#include <cstdint>
#include <vector>
#include <array>
#include <set>
#include <cstring>
#include <algorithm>
#include <stdexcept>

// ====== Configurable Oracle Committee Parameters ======
constexpr size_t NUM_ORACLES = 7;           // committee size
constexpr size_t QUORUM_THRESHOLD = 4;      // number required to sign
constexpr size_t PUBKEY_SIZE = 1472;        // bytes (Dilithium3 pubkey)
constexpr size_t SIG_SIZE = 2701;           // bytes (Dilithium3 signature)
constexpr uint64_t MIN_PRICE = 1;           // absolute lower bound (1e-15)
constexpr uint64_t MAX_PRICE = 100000000000000000ULL; // (1e17)
constexpr uint64_t MAX_TIMESTAMP_SKEW = 600; // 10 mins

// ====== Oracle Committee (pubkeys must be filled in deployment) ======
struct OraclePubKey {
    std::array<uint8_t, PUBKEY_SIZE> pubkey;
};
OraclePubKey trusted_oracles[NUM_ORACLES] = {/*...fill on deployment...*/};

// ====== Message Format ======
struct PriceMessage {
    uint64_t price;      // Fixed-point (15 decimals), network byte order for serialization
    uint64_t timestamp;  // Unix time
};

struct OracleSignature {
    size_t signer_index; // 0..NUM_ORACLES-1
    std::array<uint8_t, SIG_SIZE> signature;
};

struct PriceUpdate {
    PriceMessage message;
    std::vector<OracleSignature> signatures;
};

// ====== Quantum Signature (Dilithium) - must use side-channel-resistant code ======
extern bool dilithium_verify(
    const uint8_t* pubkey,
    const uint8_t* msg, size_t msg_len,
    const uint8_t* sig, size_t sig_len
);

// ====== Serialization, Helper ======
void serialize_message(const PriceMessage& msg, uint8_t* out16) {
    uint64_t price_net = htobe64(msg.price);
    uint64_t ts_net = htobe64(msg.timestamp);
    memcpy(out16, &price_net, 8);
    memcpy(out16 + 8, &ts_net, 8);
}

// ====== Oracle Feed State ======
struct OracleFeed {
    uint64_t last_price = 0;
    uint64_t last_timestamp = 0;
    std::vector<PriceMessage> history;
    size_t max_history = 1024;
} feed;

// ====== Get Trusted Block Timestamp ======
extern uint64_t get_current_block_timestamp(); // Use Qubic trusted source

// ====== Validate a Single Oracle Signature ======
bool validate_signature(const OraclePubKey& pubkey, const PriceMessage& msg, const std::array<uint8_t, SIG_SIZE>& sig) {
    uint8_t data[16];
    serialize_message(msg, data);
    bool ok = dilithium_verify(pubkey.pubkey.data(), data, sizeof(data), sig.data(), sig.size());
    memset(data, 0, sizeof(data));
    return ok;
}

// ====== Validate the Oracle Multi-Sig Update ======
bool validate_update(const PriceUpdate& update) {
    // A. Input: must have enough signatures, no dupes, valid signers only
    if (update.signatures.size() < QUORUM_THRESHOLD) return false;
    std::set<size_t> seen;
    size_t valid = 0;

    for (const auto& sig : update.signatures) {
        if (sig.signer_index >= NUM_ORACLES) continue;
        if (!seen.insert(sig.signer_index).second) continue; // skip dupes
        if (validate_signature(trusted_oracles[sig.signer_index], update.message, sig.signature)) {
            ++valid;
        }
    }
    if (valid < QUORUM_THRESHOLD) return false;

    // B. Monotonic timestamp, no replay
    if (update.message.timestamp <= feed.last_timestamp) return false;

    // C. Timestamp sanity (prevent far future, far past)
    uint64_t now = get_current_block_timestamp();
    if (update.message.timestamp > now + MAX_TIMESTAMP_SKEW) return false;
    if (now > 0 && update.message.timestamp + MAX_TIMESTAMP_SKEW < now) return false;

    // D. Price bounds
    if (update.message.price < MIN_PRICE || update.message.price > MAX_PRICE) return false;

    return true;
}

// ====== Submit an Oracle Price Update ======
bool submit_price_update(const PriceUpdate& update) {
    if (!validate_update(update)) return false;
    feed.last_price = update.message.price;
    feed.last_timestamp = update.message.timestamp;
    if (feed.history.size() == feed.max_history)
        feed.history.erase(feed.history.begin());
    feed.history.push_back(update.message);
    return true;
}

// ====== External Interface ======
uint64_t get_last_price()      { return feed.last_price; }
uint64_t get_last_timestamp()  { return feed.last_timestamp; }
const std::vector<PriceMessage>& get_history() { return feed.history; }

// ====== Emergency/Admin (Future, Placeholder) ======
// Could add admin multi-sig to update committee, pause contract, etc.

// ====== End ======
