// SPDX-License-Identifier: MIT
// BANKON PYTHAI Oracle Committee Example (Qubic C++ Contract)
#include <cstdint>
#include <cstring>

// Number of oracles in committee (can be increased, but 7 is a practical demo size)
constexpr uint8_t NUM_ORACLES = 7;
constexpr uint8_t SIGS_REQUIRED = 4;

// Quantum-safe public key struct (e.g., Dilithium3 pubkey)
struct OraclePubKey {
    uint8_t pubkey[1472];
};

// Quantum-safe signature (example: Dilithium3)
struct OracleSignature {
    uint8_t sig[3293];
};

// Price message structure (for one asset)
struct PriceMessage {
    int64_t price;         // Fixed-point (1e15), e.g., 1 USD = 1,000,000,000,000,000
    uint64_t timestamp;    // Unix seconds
};

// Oracle committee (stored on-chain)
OraclePubKey committee[NUM_ORACLES];
// For each oracle, their admin status (0=normal, 1=admin); first deployer is default admin
uint8_t is_admin[NUM_ORACLES];

// Current count of committee members (can be < NUM_ORACLES if keys are rotated)
uint8_t committee_size = NUM_ORACLES;

// Event for tracking last price
struct LastPrice {
    int64_t price;
    uint64_t timestamp;
} last_price;

// Only allow admin or multi-sig admin threshold for committee ops
bool is_admin_multisig(const uint8_t* signers, uint8_t num_signers) {
    uint8_t admin_count = 0;
    for (uint8_t i = 0; i < num_signers; i++) {
        uint8_t idx = signers[i];
        if (idx < committee_size && is_admin[idx]) admin_count++;
    }
    return admin_count >= SIGS_REQUIRED;
}

// Add new oracle (admin only, requires multi-sig)
bool add_oracle(const OraclePubKey& new_pk, const uint8_t* signers, uint8_t num_signers) {
    if (!is_admin_multisig(signers, num_signers)) return false;
    if (committee_size >= NUM_ORACLES) return false;
    committee[committee_size] = new_pk;
    is_admin[committee_size] = 0; // new member not admin by default
    committee_size++;
    return true;
}

// Remove oracle by index (admin only)
bool remove_oracle(uint8_t idx, const uint8_t* signers, uint8_t num_signers) {
    if (!is_admin_multisig(signers, num_signers)) return false;
    if (idx >= committee_size) return false;
    // Shift remaining oracles down
    for (uint8_t i = idx; i + 1 < committee_size; i++) {
        committee[i] = committee[i+1];
        is_admin[i] = is_admin[i+1];
    }
    committee_size--;
    return true;
}

// Rotate (replace) oracle at idx
bool rotate_oracle(uint8_t idx, const OraclePubKey& new_pk, const uint8_t* signers, uint8_t num_signers) {
    if (!is_admin_multisig(signers, num_signers)) return false;
    if (idx >= committee_size) return false;
    committee[idx] = new_pk;
    return true;
}

// Price update: require SIGS_REQUIRED valid signatures from current committee
bool verify_oracle_signatures(
    const PriceMessage& msg,
    const OracleSignature* sigs,
    const uint8_t* signer_indices,
    uint8_t num_sigs
) {
    if (num_sigs < SIGS_REQUIRED) return false;
    // For each signature, verify against committee pubkey at signer_indices[i]
    for (uint8_t i = 0; i < num_sigs; i++) {
        uint8_t idx = signer_indices[i];
        if (idx >= committee_size) return false;
        if (!verify_dilithium3_sig(msg, sigs[i], committee[idx])) return false;
    }
    return true;
}

// Core on-chain update entry (called by anyone, but only updates if signatures are valid)
bool submit_price_update(
    const PriceMessage& msg,
    const OracleSignature* sigs,
    const uint8_t* signer_indices,
    uint8_t num_sigs
) {
    if (!verify_oracle_signatures(msg, sigs, signer_indices, num_sigs)) return false;
    // Price staleness check (timestamp >= last seen, within tolerance)
    if (msg.timestamp <= last_price.timestamp || msg.timestamp > now() + 600) return false;
    last_price.price = msg.price;
    last_price.timestamp = msg.timestamp;
    return true;
}

// Placeholder for Dilithium3 signature verification (implement with your library)
bool verify_dilithium3_sig(const PriceMessage& msg, const OracleSignature& sig, const OraclePubKey& pk) {
    // TODO: Call quantum-safe verification library here
    return true;
}

// Util: On-chain time
uint64_t now() {
    // Platform/host to provide current block time
    return 0; // stub
}

// Read function to get the current on-chain price
LastPrice get_last_price() {
    return last_price;
}

/*
LICENSE BLOCK: (Insert your full license text here, as discussed previously.)
*/

