#!/bin/bash

echo "Testing BANKON PYTHAI Contract Functions..."
echo "=========================================="

# Test QX contract (contract index 1) to verify RPC works
echo "Testing QX contract query..."
QX_RESPONSE=$(curl -s -X POST "https://rpc.qubic.org/v1/querySmartContract" \
    -H "Content-Type: application/json" \
    -d '{"contractIndex": 1, "inputType": 1, "inputSize": 0, "requestData": ""}')

if [ $? -eq 0 ]; then
    echo "✅ QX contract query successful"
    echo "Response: $QX_RESPONSE"
else
    echo "❌ QX contract query failed"
fi

# Test HM25 contract (contract index 12) if available
echo "Testing HM25 contract query..."
HM25_RESPONSE=$(curl -s -X POST "https://rpc.qubic.org/v1/querySmartContract" \
    -H "Content-Type: application/json" \
    -d '{"contractIndex": 12, "inputType": 1, "inputSize": 0, "requestData": ""}')

if [ $? -eq 0 ]; then
    echo "✅ HM25 contract query successful"
    echo "Response: $HM25_RESPONSE"
else
    echo "❌ HM25 contract query failed (may not be deployed yet)"
fi

echo "Contract test completed!"
