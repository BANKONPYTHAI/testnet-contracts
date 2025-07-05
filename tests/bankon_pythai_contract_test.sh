#!/bin/bash

echo "BANKON PYTHAI Contract Test Script"
echo "=================================="

# Configuration
CONTRACT_INDEX=13
RPC_URL="https://rpc.qubic.org/v1"

# Colors for output
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Function to make RPC calls
make_rpc_call() {
    local input_type=$1
    local input_size=$2
    local request_data=$3
    local description=$4
    
    echo -e "${YELLOW}Testing: $description${NC}"
    
    response=$(curl -s -X POST "$RPC_URL/querySmartContract" \
        -H "Content-Type: application/json" \
        -d "{
            \"contractIndex\": $CONTRACT_INDEX,
            \"inputType\": $input_type,
            \"inputSize\": $input_size,
            \"requestData\": \"$request_data\"
        }")
    
    if [ $? -eq 0 ]; then
        echo -e "${GREEN}✅ Success${NC}"
        echo "Response: $response"
    else
        echo -e "${RED}❌ Failed${NC}"
        echo "Error: $response"
    fi
    echo ""
}

# Test GetTokenInfo function (input type 1)
test_get_token_info() {
    echo "=== Testing GetTokenInfo Function ==="
    make_rpc_call 1 0 "" "GetTokenInfo - Get token information"
}

# Test GetPriceFeed function (input type 2)
test_get_price_feed() {
    echo "=== Testing GetPriceFeed Function ==="
    # Test BTC price feed (asset index 0) - base64 encoded
    make_rpc_call 2 1 "AA==" "GetPriceFeed - Get BTC price feed"
    # Test ETH price feed (asset index 1) - base64 encoded
    make_rpc_call 2 1 "AQ==" "GetPriceFeed - Get ETH price feed"
}

# Test GetSyntheticAssetInfo function (input type 3)
test_get_synthetic_asset_info() {
    echo "=== Testing GetSyntheticAssetInfo Function ==="
    # Test with a sample asset ID (all zeros for now) - base64 encoded
    make_rpc_call 3 32 "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA=" "GetSyntheticAssetInfo - Get synthetic asset info"
}

# Main test function
main() {
    echo "Starting BANKON PYTHAI contract tests..."
    echo "Contract Index: $CONTRACT_INDEX"
    echo "RPC URL: $RPC_URL"
    echo ""
    
    # Check if contract is deployed
    echo "Checking if BANKON PYTHAI contract is deployed..."
    test_response=$(curl -s -X POST "$RPC_URL/querySmartContract" \
        -H "Content-Type: application/json" \
        -d "{
            \"contractIndex\": $CONTRACT_INDEX,
            \"inputType\": 1,
            \"inputSize\": 0,
            \"requestData\": \"\"
        }")
    
    if [[ $test_response == *"error"* ]] || [[ $test_response == *"not found"* ]]; then
        echo -e "${RED}❌ BANKON PYTHAI contract is not deployed yet${NC}"
        echo "Current response: $test_response"
        echo ""
        echo "To deploy the contract:"
        echo "1. Create a proposal for BANKON PYTHAI"
        echo "2. Get quorum approval (451 of 676 computors)"
        echo "3. Run IPO (Dutch auction)"
        echo "4. Deploy in epoch 153"
        return 1
    else
        echo -e "${GREEN}✅ BANKON PYTHAI contract is deployed!${NC}"
        echo ""
        
        # Run all tests
        test_get_token_info
        test_get_price_feed
        test_get_synthetic_asset_info
        
        echo -e "${GREEN}All tests completed!${NC}"
    fi
}

# Run main function
main "$@" 