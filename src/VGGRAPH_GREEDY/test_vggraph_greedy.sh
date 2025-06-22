#!/bin/bash

# Test script for VGGraph_Greedy index

echo "Building VGGraph_Greedy test executable..."
make clean
make test_vggraph_greedy.out

if [ $? -ne 0 ]; then
    echo "Build failed!"
    exit 1
fi

echo "Build successful!"

# Test basic functionality
echo "Testing basic functionality..."
./test_vggraph_greedy.out test_dataset.txt test_queries.txt 0.5 4 4

echo ""
echo "Testing with key upper bound..."
./test_vggraph_greedy.out test_dataset.txt test_queries.txt 0.3 5 4 50

echo ""
echo "Testing with lower selectivity threshold..."
./test_vggraph_greedy.out test_dataset.txt test_queries.txt 0.1 6 8 100

echo "All tests completed!"
