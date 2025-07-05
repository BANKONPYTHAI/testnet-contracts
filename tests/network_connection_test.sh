#!/bin/bash

echo "Testing Qubic Network Connection..."
echo "=================================="

# Test mainnet RPC
echo "Testing mainnet RPC..."
TICK_INFO=$(curl -s -X GET "https://rpc.qubic.org/v1/tick-info")
if [ $? -eq 0 ]; then
    echo "✅ Mainnet RPC is accessible"
    echo "Current tick info: $TICK_INFO"
else
    echo "❌ Mainnet RPC is not accessible"
fi

# Test testnet RPC
echo "Testing testnet RPC..."
TESTNET_INFO=$(curl -s -X GET "https://testnet-rpc.qubic.org/v1/tick-info")
if [ $? -eq 0 ]; then
    echo "✅ Testnet RPC is accessible"
    echo "Testnet tick info: $TESTNET_INFO"
else
    echo "❌ Testnet RPC is not accessible"
fi

# Test qubic-cli
echo "Testing qubic-cli..."
if [ -f "./qubic-cli" ]; then
    echo "✅ qubic-cli found"
    ./qubic-cli -help > /dev/null 2>&1
    if [ $? -eq 0 ]; then
        echo "✅ qubic-cli is working"
    else
        echo "❌ qubic-cli has issues"
    fi
else
    echo "❌ qubic-cli not found"
fi

echo "Connection test completed!"
