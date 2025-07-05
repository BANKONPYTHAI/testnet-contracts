;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; BANKONPYTHAI: cBTC Pool and Trustless Swap
;; Clarity contract for Stacks blockchain (.clar)
;; code is law (c) BANKON all rights reserved
;;
;; LICENSE
;; Permission is hereby granted, perpetual, worldwide, non-exclusive, free of charge, 
;; to any person obtaining a copy of this software and associated documentation files 
;; (the "Software"), to deal in the Software without restriction, including without 
;; limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, 
;; and/or sell copies of the Software, and to permit persons to whom the Software is 
;; furnished to do so, subject to the following conditions:
;;
;; [Full anti-military license text block as previously provided—see note below]
;;
;; THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
;; IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS 
;; FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR 
;; COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER 
;; IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION 
;; WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;; --- DATA STRUCTURES ---

(define-map liquidity-providers-balances principal { cbtc-balance: uint, stx-balance: uint })

;; --- CONSTANTS ---
(define-constant AUTHORITY 'ST1SJ3DTE5DN7X54YDH5D64R3BCB6A2AG2ZQ8YPD5)
(define-fungible-token cbtc)

;; --- LIQUIDITY MANAGEMENT ---

;; Add cBTC liquidity to the pool
(define-public (add-cbtc-liquidity (cbtc-sats-in uint))
  (begin
    (asserts! (> cbtc-sats-in u0) (err u10))
    ;; Transfer cBTC from user to pool
    (try! (ft-transfer? cbtc cbtc-sats-in tx-sender (as-contract tx-sender)))
    (let ((lp-balance (default-to { cbtc-balance: u0, stx-balance: u0 } (map-get? liquidity-providers-balances tx-sender)))
          (new-cbtc-balance (+ (get cbtc-balance lp-balance) cbtc-sats-in)))
      (map-set liquidity-providers-balances tx-sender { cbtc-balance: new-cbtc-balance, stx-balance: (get stx-balance lp-balance) })
      (ok { added: cbtc-sats-in, new-total: new-cbtc-balance })
    )))

;; Remove cBTC liquidity from the pool
(define-public (remove-cbtc-liquidity (cbtc-sats-out uint))
  (let ((lp-balance (default-to { cbtc-balance: u0, stx-balance: u0 } (map-get? liquidity-providers-balances tx-sender))))
    (asserts! (>= (get cbtc-balance lp-balance) cbtc-sats-out) (err u11))
    (try! (ft-transfer? cbtc cbtc-sats-out (as-contract tx-sender) tx-sender))
    (map-set liquidity-providers-balances tx-sender { cbtc-balance: (- (get cbtc-balance lp-balance) cbtc-sats-out), stx-balance: (get stx-balance lp-balance) })
    (ok { removed: cbtc-sats-out, new-total: (- (get cbtc-balance lp-balance) cbtc-sats-out) })
  ))

;; Withdraw STX from the pool (for liquidity providers)
(define-public (withdraw-stx (stx-out uint))
  (let ((lp-balance (default-to { cbtc-balance: u0, stx-balance: u0 } (map-get? liquidity-providers-balances tx-sender))))
    (asserts! (>= (get stx-balance lp-balance) stx-out) (err u12))
    (try! (stx-transfer? stx-out (as-contract tx-sender) tx-sender))
    (map-set liquidity-providers-balances tx-sender { cbtc-balance: (get cbtc-balance lp-balance), stx-balance: (- (get stx-balance lp-balance) stx-out) })
    (ok { withdrawn: stx-out, new-total: (- (get stx-balance lp-balance) stx-out) })
  ))

;; --- SWAP FUNCTIONS (TRUSTLESS, PYTH PRICING) ---

;; Simple swap using on-chain price (may be outdated, use trustless swap for prod)
(define-public (swap-stx (stx-in uint))
  (let (
        (stx-price (try! (read-stx-price-from-pyth)))
        (btc-price (try! (read-btc-price-from-pyth)))
        (sat-price (* u100000000 btc-price))
        (cbtc-sats-out (if (> stx-price u0)
                            (/ (* stx-in sat-price) stx-price)
                            u0)))
    (asserts! (> stx-in u0) (err u20))
    (asserts! (> cbtc-sats-out u0) (err u21))
    (try! (ft-transfer? cbtc cbtc-sats-out (as-contract tx-sender) tx-sender))
    (try! (stx-transfer? stx-in tx-sender (as-contract tx-sender)))
    (ok { stx-in: stx-in, cbtc-sats-out: cbtc-sats-out })
  ))

;; Trustless swap using provided price feeds, requiring user to provide Pyth proofs
(define-public (swap-stx-trustless (stx-in uint) (stx-price-feed (buff 2048)) (btc-price-feed (buff 2048)))
  (let (
        (stx-price (try! (update-and-read-stx-price-from-pyth stx-price-feed)))
        (btc-price (try! (update-and-read-btc-price-from-pyth btc-price-feed)))
        (sat-price (* u100000000 (to-uint (get price btc-price))))
        (cbtc-sats-out (if (> (to-uint (get price stx-price)) u0)
                            (/ (* stx-in sat-price) (to-uint (get price stx-price)))
                            u0)))
    (asserts! (> stx-in u0) (err u22))
    (asserts! (> cbtc-sats-out u0) (err u23))
    (try! (ft-transfer? cbtc cbtc-sats-out (as-contract tx-sender) tx-sender))
    (try! (stx-transfer? stx-in tx-sender (as-contract tx-sender)))
    (ok { stx-in: stx-in, cbtc-sats-out: cbtc-sats-out })
  ))

;; --- PYTH ORACLE INTEGRATION ---

(define-private (update-and-read-stx-price-from-pyth (stx-price-feed (buff 2048)))
  (let ((updated-prices-ids (try! 
          (contract-call? 'ST2T5JKWWP3FYYX4YRK8GK5BG2YCNGEAEY1JKX06E.pyth-oracle-v3 verify-and-update-price-feeds 
            stx-price-feed
            {
              pyth-storage-contract: 'ST2T5JKWWP3FYYX4YRK8GK5BG2YCNGEAEY1JKX06E.pyth-storage-v3,
              pyth-decoder-contract: 'ST2T5JKWWP3FYYX4YRK8GK5BG2YCNGEAEY1JKX06E.pyth-pnau-decoder-v2,
              wormhole-core-contract: 'ST2T5JKWWP3FYYX4YRK8GK5BG2YCNGEAEY1JKX06E.wormhole-core-v3
            }) )))
        (price (try! (element-at? updated-prices-ids u0))))
    (ok price)))

(define-private (update-and-read-btc-price-from-pyth (btc-price-feed (buff 2048)))
  (let ((updated-prices-ids (try! 
          (contract-call? 'ST2T5JKWWP3FYYX4YRK8GK5BG2YCNGEAEY1JKX06E.pyth-oracle-v3 verify-and-update-price-feeds 
            btc-price-feed
            {
              pyth-storage-contract: 'ST2T5JKWWP3FYYX4YRK8GK5BG2YCNGEAEY1JKX06E.pyth-storage-v3,
              pyth-decoder-contract: 'ST2T5JKWWP3FYYX4YRK8GK5BG2YCNGEAEY1JKX06E.pyth-pnau-decoder-v2,
              wormhole-core-contract: 'ST2T5JKWWP3FYYX4YRK8GK5BG2YCNGEAEY1JKX06E.wormhole-core-v3
            }) )))
        (price (try! (element-at? updated-prices-ids u0))))
    (ok price)))

(define-private (read-stx-price-from-pyth) 
  (read-price-from-pyth 0xec7a775f46379b5e943c3526b1c8d54cd49749176b0b98e02dde68d1bd335c17))

(define-private (read-btc-price-from-pyth)
  (read-price-from-pyth 0xe62df6c8b4a85fe1a67db44dc12de5db330f7ac66b72dc658afedf0f4a415b43))

(define-private (read-price-from-pyth (price-id (buff 32)))
  (let ((feed (try! (contract-call? 'ST2T5JKWWP3FYYX4YRK8GK5BG2YCNGEAEY1JKX06E.pyth-oracle-v3 read-price-feed price-id 'ST2T5JKWWP3FYYX4YRK8GK5BG2YCNGEAEY1JKX06E.pyth-storage-v3)))
        (price (get price feed)))
    (ok (to-uint price))))

;; --- SIP-010 CONFORMANCE ---

(impl-trait 'SP2J933XB2CP2JQ1A4FGN8JA968BBG3NK3EKZ7Q9F.hk-tokens-v1.sip10-token)

;; get the token balance of owner
(define-read-only (get-balance (owner principal))
  (ok (ft-get-balance cbtc owner)))

;; returns the total number of tokens
(define-read-only (get-total-supply)
  (ok (ft-get-supply cbtc)))

;; returns the token name
(define-read-only (get-name)
  (ok "cBTC"))

;; the symbol or "ticker" for this token
(define-read-only (get-symbol)
  (ok "cBTC"))

;; the number of decimals used
(define-read-only (get-decimals)
  (ok u8)) ;; 8 decimals for cBTC

;; Transfers tokens to a recipient
(define-public (transfer (amount uint) (sender principal) (recipient principal) (memo (optional (buff 34))))
  (if (is-eq tx-sender sender)
    (begin
      (try! (ft-transfer? cbtc amount sender recipient))
      (print memo)
      (ok true))
    (err u4)))

(define-public (get-token-uri)
  (ok (some u"https://bankonpythai.com/token/cbtc")))

;; --- MINT/BURN AUTHORITY FOR cBTC TOKEN (only AUTHORITY can mint) ---

(define-public (mint (amount uint) (recipient principal))
  (begin
    (asserts! (is-eq tx-sender AUTHORITY) (err u0))
    (ft-mint? cbtc amount recipient)))

(define-public (burn (amount uint))
  (begin
    (ft-burn? cbtc amount tx-sender)))

;; --- END OF CONTRACT ---

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
