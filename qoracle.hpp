/*
 * qOracle – Qubic Synthetic Oracle
 * Immutable price data feed for Qubic synthetic assets
 * Code is Law – Security First
 * License: Qubic Anti-Military, see end of file.
 */

#include <cstdint>
#include <string>
#include <unordered_map>
#include <chrono>

struct PriceData {
    uint64_t price;         // Latest price (e.g. BTC/USD, 1e8 precision for 8 decimals)
    uint64_t timestamp;     // Unix time of last update
    uint8_t decimals;       // How many decimals (e.g. 8 for BTC)
};

class QOracle {
private:
    std::unordered_map<std::string, PriceData> prices; // assetSymbol -> price
    std::string admin;
    bool adminBurned = false;

public:
    // Initialize the oracle with deployer/admin
    QOracle(const std::string& adminAddr) : admin(adminAddr) {}

    // Burn admin key, making future updates impossible
    bool burnAdmin(const std::string& sender) {
        if (adminBurned || sender != admin) return false;
        adminBurned = true;
        admin = "";
        return true;
    }

    // Push a price update (only admin, before burn)
    bool pushPrice(const std::string& assetSymbol, uint64_t newPrice, uint8_t decimals, const std::string& sender) {
        if (adminBurned || sender != admin) return false;
        prices[assetSymbol] = { newPrice, now(), decimals };
        return true;
    }

    // Get latest price for an asset (public)
    PriceData getPrice(const std::string& assetSymbol) const {
        auto it = prices.find(assetSymbol);
        if (it == prices.end()) return {0, 0, 0};
        return it->second;
    }

    // Check staleness of price
    bool isPriceFresh(const std::string& assetSymbol, uint64_t maxAgeSeconds) const {
        auto it = prices.find(assetSymbol);
        if (it == prices.end()) return false;
        return (now() - it->second.timestamp) <= maxAgeSeconds;
    }

    // Get system time (for demo; on Qubic, use contract clock)
    uint64_t now() const {
        // Use system time (for illustration – replace with Qubic's on-chain timestamp)
        return std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    }
};

/*
Qubic Anti-Military License – Code is Law Edition
Permission is hereby granted, perpetual, worldwide, non-exclusive, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

- The Software cannot be used in any form or in any substantial portions for development, maintenance and for any other purposes, in the military sphere and in relation to military products or activities as defined in the original license.
- All modifications, alterations, or merges must maintain these restrictions.
- THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND.
(c) BANKON All Rights Reserved. See LICENSE file for full text.
*/
