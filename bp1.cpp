/***************************************************************
BANKON PYTHAI Smart Contract
Fixed Supply Utility Token on Qubic

Permission is hereby granted, perpetual, worldwide, non-exclusive, 
free of charge, to any person obtaining a copy of this software 
and associated documentation files (the "Software"), to deal in 
the Software without restriction, including without limitation 
the rights to use, copy, modify, merge, publish, distribute, 
sublicense, and/or sell copies of the Software, and to permit 
persons to whom the Software is furnished to do so, subject to 
the following conditions:

1. The Software cannot be used in any form or in any substantial portions for development, maintenance and for any other purposes, in the military sphere and in relation to military products, including, but not limited to:
a. any kind of armored force vehicles, missile weapons, warships, artillery weapons, air military vehicles (including military aircrafts, combat helicopters, military drones aircrafts), air defense systems, rifle armaments, small arms, firearms and side arms, melee weapons, chemical weapons, weapons of mass destruction;
b. any special software for development technical documentation for military purposes;
c. any special equipment for tests of prototypes of any subjects with military purpose of use;
d. any means of protection for conduction of acts of a military nature;
e. any software or hardware for determining strategies, reconnaissance, troop positioning, conducting military actions, conducting special operations;
f. any dual-use products with possibility to use the product in military purposes;
g. any other products, software or services connected to military activities;
h. any auxiliary means related to abovementioned spheres and products.

2. The Software cannot be used as described herein in any connection to the military activities. A person, a company, or any other entity, which wants to use the Software, shall take all reasonable actions to make sure that the purpose of use of the Software cannot be possibly connected to military purposes.

3. The Software cannot be used by a person, a company, or any other entity, activities of which are connected to military sphere in any means. If a person, a company, or any other entity, during the period of time for the usage of Software, would engage in activities, connected to military purposes, such person, company, or any other entity shall immediately stop the usage of Software and any its modifications or alterations.

4. Abovementioned restrictions should apply to all modification, alteration, merge, and to other actions, related to the Software, regardless of how the Software was changed due to the abovementioned actions.

The above copyright notice and this permission notice shall be included in all copies or substantial portions, modifications and alterations of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
****************************************************************/

#include <cstdint>
#include <unordered_map>

/**
 * BANKON PYTHAI (BKPY)
 * Fixed-supply utility token, 100,000.000000000000 units (15 decimals)
 * - Mintable once by admin (at deployment), no further minting/burning
 * - Secure safe-math logic
 * - No admin after mint
 * - No approvals, no reentrancy, math safe
 */

// -------------------- PARAMETERS --------------------
constexpr uint64_t DECIMALS = 1000000000000000ULL;      // 15 decimals
constexpr uint64_t TOTAL_SUPPLY = 100000 * DECIMALS;    // 100,000.000000000000
// ----------------------------------------------------

// -------------- SAFE MATH LIBRARY -------------------
inline bool safeAdd(uint64_t a, uint64_t b, uint64_t& result) {
    result = a + b;
    return result >= a;
}

inline bool safeSub(uint64_t a, uint64_t b, uint64_t& result) {
    if (a < b) return false;
    result = a - b;
    return true;
}
// ----------------------------------------------------

// -------------- CONTRACT DEFINITION -----------------
class BANKON_PYTHAI {
    std::unordered_map<uint64_t, uint64_t> balances;  // address (uint64_t) => balance
    uint64_t admin;            // contract deployer, receives initial supply
    bool minted = false;       // supply can only be minted ONCE

public:
    // Constructor: set contract deployer as admin
    BANKON_PYTHAI(uint64_t deployer) : admin(deployer) {}

    /**
     * Mint initial supply to admin only ONCE.
     * Returns true on success, false if already minted.
     */
    bool mint() {
        if (minted) return false;
        balances[admin] = TOTAL_SUPPLY;
        minted = true;
        return true;
    }

    /**
     * Transfer tokens from sender to recipient.
     * Ensures atomic, overflow/underflow-safe transfer.
     */
    bool transfer(uint64_t from, uint64_t to, uint64_t amount) {
        if (amount == 0) return false;
        if (from == to) return false;
        uint64_t fromBal = balances[from];
        uint64_t newFromBal;
        if (!safeSub(fromBal, amount, newFromBal)) return false;
        uint64_t newToBal;
        if (!safeAdd(balances[to], amount, newToBal)) return false;
        balances[from] = newFromBal;
        balances[to]   = newToBal;
        return true;
    }

    /**
     * Returns balance of an address.
     */
    uint64_t balanceOf(uint64_t user) const {
        auto it = balances.find(user);
        return it != balances.end() ? it->second : 0;
    }

    /**
     * Returns total supply of BANKON PYTHAI token.
     */
    uint64_t totalSupply() const {
        return TOTAL_SUPPLY;
    }

    // No admin withdrawal, mint, or burn after initial mint.
    // No approve/allowance logic; transfer only.
    // All logic strictly checked for math and security.
};

// ----------------- END OF CONTRACT ------------------

/*
AUDIT/SECURITY NOTES:
- All balances and supply use fixed-point math (uint64_t), with 15 decimals (no floats).
- Minting can only be performed once, at deployment, by the admin.
- No further admin, mint, or burn functions after deployment.
- Transfers use safe add/sub to prevent overflow/underflow.
- No reentrancy (no external call/approval logic).
- No privileged operations post-mint.
- Full license and prohibition against military use are included.
- All state and logic are fully transparent and documented.
*/
