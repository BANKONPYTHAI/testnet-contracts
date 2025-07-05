/*
 * Qnosis – Qubic Multisig Safe/Vault
 * N-of-M Quantum-Ready, Immutable, Secure, and Simple
 * (c) BANKON All Rights Reserved — Qubic Anti-Military License
 */

#include <vector>
#include <unordered_map>
#include <string>
#include <set>
#include <algorithm>
#include <cstdint>

class Qnosis {
public:
    // Represents a transaction proposal
    struct Proposal {
        std::string to;            // destination address (or contract)
        uint64_t value;            // amount (for transfers, in qBTC or QUBIC units)
        std::string data;          // payload (for contract call)
        uint64_t nonce;            // unique identifier
        bool executed;             // has the proposal been executed?
        std::set<std::string> sigs; // set of owner addresses that have signed
        std::string action;        // "transfer", "add-owner", "remove-owner", etc.
        std::string param;         // parameter for action (e.g., owner address for add/remove)
    };

    std::vector<std::string> owners;                   // Current owners
    uint32_t threshold;                                // Signatures required
    uint64_t proposalNonce;                            // For replay protection
    std::unordered_map<uint64_t, Proposal> proposals;  // Map nonce => proposal

    // ==== Constructor ====
    Qnosis(const std::vector<std::string>& initialOwners, uint32_t thresh) {
        require(initialOwners.size() > 0, "Owners required");
        require(thresh > 0 && thresh <= initialOwners.size(), "Invalid threshold");
        owners = initialOwners;
        threshold = thresh;
        proposalNonce = 1;
    }

    // ==== Utility: Checks ====
    bool isOwner(const std::string& addr) const {
        return std::find(owners.begin(), owners.end(), addr) != owners.end();
    }

    void require(bool cond, const std::string& msg) const {
        if (!cond) throw std::runtime_error(msg);
    }

    // ==== Proposal Management ====

    // Propose new transaction (transfer or contract call)
    uint64_t propose(const std::string& proposer, const std::string& to, uint64_t value, const std::string& data, const std::string& action = "transfer", const std::string& param = "") {
        require(isOwner(proposer), "Not an owner");
        Proposal p{to, value, data, proposalNonce, false, {}, action, param};
        proposals[proposalNonce] = p;
        proposalNonce++;
        return proposalNonce - 1;
    }

    // Sign a proposal (no double signing)
    void sign(uint64_t nonce, const std::string& signer) {
        require(isOwner(signer), "Not an owner");
        require(proposals.count(nonce), "No such proposal");
        Proposal& p = proposals[nonce];
        require(!p.executed, "Already executed");
        p.sigs.insert(signer);
    }

    // Can this proposal be executed?
    bool canExecute(uint64_t nonce) const {
        auto it = proposals.find(nonce);
        if (it == proposals.end()) return false;
        const Proposal& p = it->second;
        return !p.executed && p.sigs.size() >= threshold;
    }

    // Execute proposal (must be signed by threshold)
    void execute(uint64_t nonce) {
        require(proposals.count(nonce), "No such proposal");
        Proposal& p = proposals[nonce];
        require(!p.executed, "Already executed");
        require(p.sigs.size() >= threshold, "Not enough signatures");

        if (p.action == "transfer") {
            // [Insert logic for Qubic-native transfer/call, e.g.]:
            // Token::transfer(address(this), p.to, p.value);
            // Or: QUBIC::transfer(p.to, p.value);
            // Or: Contract::call(p.to, p.data);
            // (In real Qubic, must integrate with chain/call API)
        }
        else if (p.action == "add-owner") {
            require(!isOwner(p.param), "Already an owner");
            owners.push_back(p.param);
        }
        else if (p.action == "remove-owner") {
            require(isOwner(p.param), "Not an owner");
            require(owners.size() > 1, "At least 1 owner required");
            owners.erase(std::remove(owners.begin(), owners.end(), p.param), owners.end());
            if (threshold > owners.size()) threshold = owners.size(); // Adjust threshold if needed
        }
        else if (p.action == "change-threshold") {
            uint32_t newThresh = std::stoul(p.param);
            require(newThresh > 0 && newThresh <= owners.size(), "Invalid threshold");
            threshold = newThresh;
        }
        // Quantum signature logic: can be plugged in here (or when validating signatures)
        p.executed = true;
    }

    // ==== View/Info ====
    std::vector<std::string> getOwners() const { return owners; }
    uint32_t getThreshold() const { return threshold; }
    bool isExecuted(uint64_t nonce) const { return proposals.count(nonce) ? proposals.at(nonce).executed : false; }
    std::set<std::string> getSignatures(uint64_t nonce) const { return proposals.count(nonce) ? proposals.at(nonce).sigs : std::set<std::string>{}; }
    Proposal getProposal(uint64_t nonce) const { require(proposals.count(nonce), "No such proposal"); return proposals.at(nonce); }
};

