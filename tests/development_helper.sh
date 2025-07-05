#!/bin/bash

echo "BANKON PYTHAI Development Helper"
echo "================================"

case "$1" in
    "status")
        echo "Current Network Status:"
        curl -s -X GET "https://rpc.qubic.org/v1/tick-info" | jq .
        ;;
    "test")
        ./scripts/test_connection.sh
        ./scripts/test_contract.sh
        ;;
    "deploy")
        echo "Deploy BANKON PYTHAI contract (placeholder)"
        echo "This would involve:"
        echo "1. Creating a proposal"
        echo "2. Getting quorum approval"
        echo "3. Running IPO"
        echo "4. Deploying contract"
        ;;
    "interact")
        echo "Interact with BANKON PYTHAI contract (placeholder)"
        echo "Available functions:"
        echo "- GetTokenInfo"
        echo "- Transfer"
        echo "- GetPriceFeed"
        echo "- UpdatePriceFeed"
        echo "- CreateSyntheticAsset"
        echo "- MintSyntheticAsset"
        echo "- GetSyntheticAssetInfo"
        ;;
    *)
        echo "Usage: $0 {status|test|deploy|interact}"
        echo ""
        echo "Commands:"
        echo "  status   - Show current network status"
        echo "  test     - Run connection and contract tests"
        echo "  deploy   - Deploy BANKON PYTHAI contract"
        echo "  interact - Interact with BANKON PYTHAI contract"
        ;;
esac
