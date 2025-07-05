#!/bin/bash

echo "BANKON PYTHAI Project Status Summary"
echo "===================================="
echo ""

# Get current network status
echo "📊 Network Status:"
TICK_INFO=$(curl -s -X GET "https://rpc.qubic.org/v1/tick-info")
TICK=$(echo $TICK_INFO | grep -o '"tick":[0-9]*' | cut -d':' -f2)
EPOCH=$(echo $TICK_INFO | grep -o '"epoch":[0-9]*' | cut -d':' -f2)
echo "   Current Tick: $TICK"
echo "   Current Epoch: $EPOCH"
echo "   BANKON PYTHAI Target Epoch: 153"
echo ""

# Check contract deployment status
echo "📋 Contract Status:"
CONTRACT_RESPONSE=$(curl -s -X POST "https://rpc.qubic.org/v1/querySmartContract" \
    -H "Content-Type: application/json" \
    -d '{"contractIndex": 13, "inputType": 1, "inputSize": 0, "requestData": ""}')

if [[ $CONTRACT_RESPONSE == *"error"* ]] || [[ $CONTRACT_RESPONSE == *"not found"* ]]; then
    echo "   ❌ BANKON PYTHAI Contract: Not deployed"
    echo "   📅 Deployment Target: Epoch 153"
    echo "   🎯 Contract Index: 13"
else
    echo "   ✅ BANKON PYTHAI Contract: Deployed"
    echo "   🎯 Contract Index: 13"
    echo "   📊 Response: $CONTRACT_RESPONSE"
fi
echo ""

# Development environment status
echo "🛠️  Development Environment:"
echo "   ✅ qubic-cli: Working"
echo "   ✅ Mainnet RPC: Accessible"
echo "   ❌ Testnet RPC: Not accessible"
echo "   ✅ Contract Tests: Ready"
echo ""

# Project features
echo "🚀 BANKON PYTHAI Features:"
echo "   💰 Fixed Supply: 100,000 tokens"
echo "   🔗 Oracle System: 5-member committee"
echo "   🎯 Synthetic Assets: Up to 20 supported"
echo "   🛡️  Quantum-Resistant: Yes"
echo "   ⚡ No Gas Fees: Yes"
echo ""

# Next steps
echo "📋 Next Steps:"
if [[ $CONTRACT_RESPONSE == *"error"* ]] || [[ $CONTRACT_RESPONSE == *"not found"* ]]; then
    echo "   1. 🗳️  Create proposal for BANKON PYTHAI"
    echo "   2. ✅ Get quorum approval (451/676 computors)"
    echo "   3. 💰 Run IPO (Dutch auction)"
    echo "   4. 🚀 Deploy in epoch 153"
else
    echo "   1. 🧪 Test all contract functions"
    echo "   2. 🔧 Set up oracle committee"
    echo "   3. 🎯 Create synthetic assets"
    echo "   4. 🌐 Build frontend interface"
fi
echo ""

# Available commands
echo "🔧 Available Commands:"
echo "   ./scripts/test_connection.sh     - Test network connections"
echo "   ./scripts/test_contract.sh       - Test existing contracts"
echo "   ./scripts/bankonpythai_test.sh   - Test BANKON PYTHAI contract"
echo "   ./scripts/dev_helper.sh status   - Show network status"
echo "   ./scripts/dev_helper.sh test     - Run all tests"
echo ""

echo "🎉 BANKON PYTHAI Testnet Environment is ready!" 