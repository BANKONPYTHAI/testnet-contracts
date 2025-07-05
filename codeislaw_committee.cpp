/**************************************************************************************
* BANKON PYTHAI Oracle Committee Smart Contract
* License: See full license text below.
*
* Summary:
* - Immutable, rule-based oracle price feed.
* - Committee is set at deployment; never changes.
* - Only valid, signed updates accepted.
* - Data stored on-chain, queryable by all.
* - No admin, upgrade, or external intervention.
**************************************************************************************/

#include <cstdint>
#include <array>
#include <algorithm>
#include <cstring>

// ---- Configuration ----
constexpr uint32_t NUM_ORACLES = 7;  // set at deployment
constexpr uint32_t MAJORITY = 4;     // >50% required

// All keys are public and set at deployment; cannot change after!
const std::array<uint8_t[32], NUM_ORACLES> ORACLE_PUBKEYS = {
    /* Fill with oracle pubkeys */
};

// Precision: 15 decimals, fixed-point
constexpr uint64_t DECIMALS = 1000000000000000ULL;

// ---- State ----
struct OracleUpdate {
    uint64_t value;      // e.g., price * DECIMALS
    uint32_t timestamp;  // unix time
    uint8_t signatures[NUM_ORACLES][64]; // Ed25519 sigs
    uint8_t signers[NUM_ORACLES];        // which oracle signed (index)
    uint8_t count;                       // how many sigs present
};

OracleUpdate lastUpdate;

// ---- Utility ----

// Verify Ed25519 sig (pseudo-function; replace w/ Qubic's actual signature verify)
bool verify_ed25519(
    const uint8_t pubkey[32], const uint8_t *msg, uint32_t msglen, const uint8_t sig[64]) {
    // ... platform-specific implementation
    return true; // stub
}

// Check signer is in committee
bool is_valid_oracle(const uint8_t pubkey[32]) {
    for (const auto& k : ORACLE_PUBKEYS)
        if (memcmp(pubkey, k, 32) == 0) return true;
    return false;
}

// Check all signers are unique and valid
bool check_signers(const OracleUpdate& upd) {
    std::array<bool, NUM_ORACLES> seen{};
    for (uint8_t i = 0; i < upd.count; ++i) {
        uint8_t idx = upd.signers[i];
        if (idx >= NUM_ORACLES || seen[idx]) return false;
        seen[idx] = true;
    }
    return true;
}

// ---- Core Function: Submit Oracle Value ----
extern "C" int submit_update(
    const uint64_t value,
    const uint32_t timestamp,
    const uint8_t signatures[NUM_ORACLES][64],
    const uint8_t signers[NUM_ORACLES],
    const uint8_t count
) {
    // Only accept new updates
    if (timestamp <= lastUpdate.timestamp)
        return -1; // outdated or duplicate

    // Validate enough signatures
    if (count < MAJORITY || count > NUM_ORACLES)
        return -2; // not enough sigs

    // Check signers and sigs
    OracleUpdate upd = {};
    upd.value = value;
    upd.timestamp = timestamp;
    upd.count = count;
    memcpy(upd.signers, signers, count);

    if (!check_signers(upd))
        return -3; // bad signers

    // Build message (value + timestamp, 12 bytes)
    uint8_t msg[12];
    memcpy(msg, &value, 8);
    memcpy(msg+8, &timestamp, 4);

    // Validate each signature
    for (uint8_t i = 0; i < count; ++i) {
        uint8_t idx = signers[i];
        if (idx >= NUM_ORACLES) return -4;
        if (!verify_ed25519(ORACLE_PUBKEYS[idx], msg, 12, signatures[i]))
            return -5; // sig fail
        memcpy(upd.signatures[i], signatures[i], 64);
    }

    // Majority confirmed, update state
    lastUpdate = upd;
    return 0;
}

// ---- Core Function: Read Oracle Value ----
extern "C" int read_oracle(uint64_t* value, uint32_t* timestamp) {
    *value = lastUpdate.value;
    *timestamp = lastUpdate.timestamp;
    return 0;
}

/**************************************************************************************
 * LICENSE
 *
 * Permission is hereby granted, perpetual, worldwide, non-exclusive, free of charge,
 * to any person obtaining a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction, including without
 * limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The Software cannot be used in any form or in any substantial portions for
 * development, maintenance and for any other purposes, in the military sphere and in
 * relation to military products, including, but not limited to: any kind of armored
 * force vehicles, missile weapons, warships, artillery weapons, air military vehicles
 * (including military aircrafts, combat helicopters, military drones aircrafts), air
 * defense systems, rifle armaments, small arms, firearms and side arms, melee weapons,
 * chemical weapons, weapons of mass destruction; any special software for development
 * technical documentation for military purposes; any special equipment for tests of
 * prototypes of any subjects with military purpose of use; any means of protection for
 * conduction of acts of a military nature; any software or hardware for determining
 * strategies, reconnaissance, troop positioning, conducting military actions,
 * conducting special operations; any dual-use products with possibility to use the
 * product in military purposes; any other products, software or services connected to
 * military activities; any auxiliary means related to abovementioned spheres and
 * products.
 *
 * The Software cannot be used as described herein in any connection to the military
 * activities. A person, a company, or any other entity, which wants to use the
 * Software, shall take all reasonable actions to make sure that the purpose of use of
 * the Software cannot be possibly connected to military purposes.
 *
 * The Software cannot be used by a person, a company, or any other entity, activities
 * of which are connected to military sphere in any means. If a person, a company, or
 * any other entity, during the period of time for the usage of Software, would engage
 * in activities, connected to military purposes, such person, company, or any other
 * entity shall immediately stop the usage of Software and any its modifications or
 * alterations.
 *
 * Abovementioned restrictions should apply to all modification, alteration, merge,
 * and to other actions, related to the Software, regardless of how the Software was
 * changed due to the abovementioned actions.
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions, modifications and alterations of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 **************************************************************************************/

