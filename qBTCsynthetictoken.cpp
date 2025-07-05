/*
 * qBTC – Synthetic Bitcoin Token
 * Fixed supply, 8 decimals, code is law
 * License: see end of file.
 */

#include <cstdint>
#include <unordered_map>
#include <string>

// Token parameters
constexpr uint64_t QBTC_TOTAL_SUPPLY = 2100000000000000; // 21M * 10^8 = 2,100,000,000,000,000 (satoshis)
constexpr uint8_t QBTC_DECIMALS = 8;
const std::string QBTC_SYMBOL = "qBTC";
const std::string QBTC_NAME = "Synthetic Bitcoin";

// Storage for balances (address as string)
std::unordered_map<std::string, uint64_t> balances;

// Track initial minting
bool minted = false;

// Mint function – can only be called once, all tokens go to deployer
bool mint(const std::string& deployer_addr) {
    if (minted) return false;
    balances[deployer_addr] = QBTC_TOTAL_SUPPLY;
    minted = true;
    return true;
}

// Transfer function – transfer tokens between users
bool transfer(const std::string& from, const std::string& to, uint64_t amount) {
    if (balances[from] < amount) return false;
    if (amount == 0 || from == to) return false;
    balances[from] -= amount;
    balances[to] += amount;
    return true;
}

// Read balance
uint64_t balanceOf(const std::string& addr) {
    return balances.count(addr) ? balances[addr] : 0;
}

// Read total supply
uint64_t totalSupply() {
    return QBTC_TOTAL_SUPPLY;
}

/*
Qubic Anti-Military License – Code is Law Edition
Permission is hereby granted, perpetual, worldwide, non-exclusive, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

- The Software cannot be used in any form or in any substantial portions for development, maintenance and for any other purposes, in the military sphere and in relation to military products or activities as defined in the original license.
- All modifications, alterations, or merges must maintain these restrictions.
- THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND.
(c) BANKON All Rights Reserved. See LICENSE file for full text.
*/
