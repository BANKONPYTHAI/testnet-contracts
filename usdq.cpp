/*
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

// USDq - Qubic Smart Contract - 1:1 Synthetic USDC Peg, No Fees

#include <cstdint>
#include <map>
#include <vector>

// 15 decimals of precision (fixed point math)
const uint8_t DECIMALS = 15;
const uint64_t DECIMAL_MULTIPLIER = 1000000000000000ULL;

// No max supply: mirrors bridged/minted USDC
const uint64_t MAX_SUPPLY = UINT64_MAX;

// Set this to your bridge/custodian public key
const uint8_t AUTHORIZED_MINT_BURN_PUBKEY[32] = { /* Fill in custodian key */ };

// Storage for balances
std::map<std::vector<uint8_t>, uint64_t> balances;
uint64_t totalSupply = 0;

// Input/output structs
struct TransferInput {
    uint8_t to[32];
    uint64_t amount; // Fixed-point, 15 decimals
};

struct BalanceOfInput {
    uint8_t account[32];
};
struct BalanceOfOutput {
    uint64_t balance;
};

struct TotalSupplyOutput {
    uint64_t totalSupply;
};

struct MintBurnInput {
    uint8_t to_or_from[32];
    uint64_t amount;
};

// Permission check
bool isAuthorized(const uint8_t pubkey[32]) {
    for (int i = 0; i < 32; ++i)
        if (pubkey[i] != AUTHORIZED_MINT_BURN_PUBKEY[i])
            return false;
    return true;
}

// Safe Math
bool safeAdd(uint64_t a, uint64_t b, uint64_t &result) {
    result = a + b;
    return result >= a;
}
bool safeSub(uint64_t a, uint64_t b, uint64_t &result) {
    if (b > a) return false;
    result = a - b;
    return true;
}

// Mint: only authorized bridge/custodian may mint
extern "C" void mint(const MintBurnInput& input, const uint8_t caller[32]) {
    if (!isAuthorized(caller)) return;
    if (input.amount == 0) return;

    uint64_t newBalance, newSupply;
    std::vector<uint8_t> to(input.to_or_from, input.to_or_from + 32);

    if (!safeAdd(balances[to], input.amount, newBalance)) return;
    if (!safeAdd(totalSupply, input.amount, newSupply)) return;

    balances[to] = newBalance;
    totalSupply = newSupply;
}

// Burn: only authorized bridge/custodian may burn
extern "C" void burn(const MintBurnInput& input, const uint8_t caller[32]) {
    if (!isAuthorized(caller)) return;
    if (input.amount == 0) return;

    std::vector<uint8_t> from(input.to_or_from, input.to_or_from + 32);
    uint64_t userBalance = balances[from];

    if (userBalance < input.amount) return;

    uint64_t newBalance, newSupply;
    if (!safeSub(userBalance, input.amount, newBalance)) return;
    if (!safeSub(totalSupply, input.amount, newSupply)) return;

    balances[from] = newBalance;
    totalSupply = newSupply;
}

// Transfer: no fees, standard move
extern "C" void transfer(const TransferInput& input, const uint8_t sender[32]) {
    if (input.amount == 0) return;
    std::vector<uint8_t> from(sender, sender + 32);
    std::vector<uint8_t> to(input.to, input.to + 32);

    if (from == to) return;

    uint64_t fromBalance = balances[from];
    if (fromBalance < input.amount) return;

    uint64_t newFrom, newTo;
    if (!safeSub(fromBalance, input.amount, newFrom)) return;
    if (!safeAdd(balances[to], input.amount, newTo)) return;

    balances[from] = newFrom;
    balances[to] = newTo;
}

// balanceOf
extern "C" BalanceOfOutput balanceOf(const BalanceOfInput& input) {
    std::vector<uint8_t> account(input.account, input.account + 32);
    BalanceOfOutput output = { balances[account] };
    return output;
}

// totalSupply
extern "C" TotalSupplyOutput getTotalSupply() {
    TotalSupplyOutput output = { totalSupply };
    return output;
}
