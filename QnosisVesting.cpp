/*
 * QnosisVestingPool – Qubic Multi-Sig Vesting with Code-is-Law
 * (c) BANKON All Rights Reserved — Qubic Anti-Military License
 */

#include <unordered_map>
#include <vector>
#include <string>
#include <cstdint>
#include <cassert>

// ---- Config ----
static constexpr uint8_t MAX_SIGNERS = 10;
static constexpr uint8_t THRESHOLD = 3; // Min signatures needed

// ---- Qubic Qnosis Multisig State ----
std::vector<std::string> signers;      // Public keys (or addresses) of multisig
std::unordered_map<std::string, bool> isSigner; // Quick lookup

// ---- ERC20-style token state ----
std::unordered_map<std::string, uint64_t> balances;
uint64_t totalSupply = 0;

// ---- Vesting State ----
struct Vesting {
    std::string beneficiary;
    uint64_t totalAmount;
    uint64_t startTime;      // UNIX timestamp
    uint64_t duration;       // seconds
    uint64_t claimedAmount;
    bool paused;
    bool cancelled;
};

std::unordered_map<std::string, Vesting> vestings;

// ---- Events (pseudo, replace with real Qubic events/syscalls) ----
void emitEvent(const std::string &event, const std::string &data) {
    // Qubic: log or trigger event in your system.
}

// ---- Qnosis: Add signer (only contract deployer or Qnosis group) ----
void addSigner(const std::string &newSigner, const std::vector<std::string> &multisigProof) {
    assert(signers.size() < MAX_SIGNERS);
    assert(isAuthorized(multisigProof));
    signers.push_back(newSigner);
    isSigner[newSigner] = true;
    emitEvent("AddSigner", newSigner);
}

// ---- Qnosis: Remove signer ----
void removeSigner(const std::string &oldSigner, const std::vector<std::string> &multisigProof) {
    assert(isSigner[oldSigner]);
    assert(isAuthorized(multisigProof));
    for (auto it = signers.begin(); it != signers.end(); ++it) {
        if (*it == oldSigner) {
            signers.erase(it);
            break;
        }
    }
    isSigner[oldSigner] = false;
    emitEvent("RemoveSigner", oldSigner);
}

// ---- Qnosis: Check Multisig Proof (threshold signatures) ----
bool isAuthorized(const std::vector<std::string> &proof) {
    uint8_t valid = 0;
    std::unordered_map<std::string, bool> seen;
    for (const auto &sig : proof) {
        if (isSigner[sig] && !seen[sig]) {
            valid++;
            seen[sig] = true;
        }
    }
    return valid >= THRESHOLD;
}

// ---- Mint Tokens (for Vesting) ----
void mint(const std::string &to, uint64_t amount, const std::vector<std::string> &multisigProof) {
    assert(isAuthorized(multisigProof));
    balances[to] += amount;
    totalSupply += amount;
    emitEvent("Mint", to);
}

// ---- Create Vesting ----
void createVesting(
    const std::string &id,
    const std::string &beneficiary,
    uint64_t totalAmount,
    uint64_t startTime,
    uint64_t duration,
    const std::vector<std::string> &multisigProof
) {
    assert(isAuthorized(multisigProof));
    assert(balances[beneficiary] >= totalAmount);
    assert(vestings.count(id) == 0); // Unique vesting ID
    Vesting v = {beneficiary, totalAmount, startTime, duration, 0, false, false};
    vestings[id] = v;
    balances[beneficiary] -= totalAmount; // Lock tokens for vesting
    emitEvent("CreateVesting", id);
}

// ---- Claim Vesting ----
void claimVesting(const std::string &id, const std::string &caller) {
    assert(vestings.count(id));
    Vesting &v = vestings[id];
    assert(v.beneficiary == caller);
    assert(!v.paused && !v.cancelled);
    uint64_t now = getCurrentTimestamp(); // Qubic syscall to get block time
    uint64_t elapsed = (now > v.startTime) ? now - v.startTime : 0;
    uint64_t vested = (elapsed >= v.duration) ? v.totalAmount : (v.totalAmount * elapsed / v.duration);
    uint64_t claimable = vested - v.claimedAmount;
    assert(claimable > 0);
    v.claimedAmount += claimable;
    balances[caller] += claimable;
    emitEvent("ClaimVesting", id);
}

// ---- Pause/Unpause/Cancellation (Qnosis only) ----
void pauseVesting(const std::string &id, const std::vector<std::string> &multisigProof) {
    assert(isAuthorized(multisigProof));
    assert(vestings.count(id));
    vestings[id].paused = true;
    emitEvent("PauseVesting", id);
}
void unpauseVesting(const std::string &id, const std::vector<std::string> &multisigProof) {
    assert(isAuthorized(multisigProof));
    assert(vestings.count(id));
    vestings[id].paused = false;
    emitEvent("UnpauseVesting", id);
}
void cancelVesting(const std::string &id, const std::vector<std::string> &multisigProof) {
    assert(isAuthorized(multisigProof));
    assert(vestings.count(id));
    vestings[id].cancelled = true;
    emitEvent("CancelVesting", id);
}

// ---- Util: Qubic timestamp syscall stub ----
uint64_t getCurrentTimestamp() {
    // TODO: Replace with Qubic system call for block timestamp
    return 0;
}
