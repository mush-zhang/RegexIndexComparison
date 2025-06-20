#!/bin/bash
# Test script to verify VGGraph_Opt respects key_upper_bound_

echo "Testing VGGraph_Opt Key Upper Bound Implementation"
echo "================================================="

cd /Users/lzhang55/Documents/GitHub/RegexIndexComparison/src/VGGRAPH_OPT

# Build VGGraph_Opt
echo "Building VGGraph_Opt..."
make clean
make test_vggraph_opt.out

if [ $? -ne 0 ]; then
    echo "Build failed!"
    exit 1
fi

echo "Build successful!"
echo ""

# Test with different key upper bounds
echo "Testing with different key upper bounds:"

echo ""
echo "Test 1: Upper bound = 5"
echo "----------------------"
./test_vggraph_opt.out test_dataset.txt test_queries.txt 0.8 3 2 5

echo ""
echo "Test 2: Upper bound = 10"
echo "-----------------------"
./test_vggraph_opt.out test_dataset.txt test_queries.txt 0.8 3 2 10

echo ""
echo "Test 3: Upper bound = 20"
echo "-----------------------"
./test_vggraph_opt.out test_dataset.txt test_queries.txt 0.8 3 2 20

echo ""
echo "Test 4: Very small bound = 2"
echo "-----------------------------"
./test_vggraph_opt.out test_dataset.txt test_queries.txt 0.8 3 2 2

echo ""
echo "Key upper bound testing completed!"
