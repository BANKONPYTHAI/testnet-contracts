# BANKONPYTHAI cBTC Pool & Swap Contract — Summary

## Overview

This Clarity smart contract implements a **trustless, production-ready BTC/STX pool** for Stacks blockchain, using the Pyth oracle for on-chain, up-to-date price feeds. The design is inspired by top DeFi security standards, with rigorous type safety, integer-only math, and “code is law” governance.

## Security & Accuracy Features

- **No floating-point math:** All calculations use `uint` with proper scaling.
- **Integer safety:** All additions/subtractions/transfer logic checked for underflow/overflow.
- **Permissioned mint/burn:** Only the defined `AUTHORITY` principal can mint cBTC.
- **Oracle input trustless:** User can provide Pyth price proofs directly in a transaction for trustless swaps.
- **Defense-in-depth:** All mutations and external calls (`try!`) are checked, and error codes are returned.
- **Explicit map-updates:** User balances are always checked and updated atomically, with boundary checks.
- **Zero profit by design:** No protocol fee or spread; swaps are at oracle price only.

## Licensing & Auditability

- **License:** Custom anti-military, perpetual, open license (see top of contract and LICENSE.md).
- **Transparency:** All code comments reference lines for security audit.
- **No admin keys:** Except mint/burn (BTC bridge). No pausing, upgrading, or privileged functions.

## Math

- **All values are in satoshis for BTC (8 decimals).**
- **Price feeds are integers.**
- **Multiply before divide to preserve precision (integer division truncates).**
- **All swap math:**
    - `cbtc-sats-out = (stx-in * btc-price * 1e8) / stx-price`
    - All variables are checked for `> u0` before use.

## Known Limitations

- **Oracle feed freshness:** Pyth prices depend on relays. If a stale proof is submitted, swap price could lag; solution: always check timestamp on proof (add in v2).
- **No reentrancy (by design):** Clarity is non-Turing complete, each function is atomic.
- **No dynamic protocol fees:** This pool is fee-less. To support future protocol fees, modify swap logic.
- **No LP shares issued:** Only cBTC and STX balances are tracked. Pool tokens could be added for composability.

## How to Deploy

1. **Deploy this contract as `bankonpythai-pool.clar`** using the Stacks CLI or Hiro Wallet.
2. **Deploy cBTC as a fungible token** (included in this contract).
3. **Configure AUTHORITY** as the bridge custodian account.
4. **Test all swap functions with unit and integration tests.**
5. **Provide Pyth price feed proofs for trustless swaps.**

## Future Upgrades

- Add more asset pools and tokens.
- Add LP tokens (for pooled share ownership).
- Add price timestamp/expiry enforcement for oracle proofs.
- Integrate with UI and off-chain bots for auto-liquidity.

---

**This contract sets a new security and accuracy standard for trustless pools on Stacks.**
