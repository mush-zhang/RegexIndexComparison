#!/bin/bash
# Test script for VGGraph_Opt index

echo "Testing VGGraph_Opt Index Implementation"
echo "========================================"

# Build the VGGraph_Opt index
echo "Building VGGraph_Opt index..."
cd src/VGGRAPH_OPT
make clean
make test_vggraph_opt.out

if [ $? -eq 0 ]; then
    echo "Build successful!"
    echo ""
    echo "Running test with parameters:"
    echo "  Dataset: test_dataset.txt"
    echo "  Queries: test_queries.txt"
    echo "  Selectivity threshold: 0.5"
    echo "  Upper n: 3"
    echo "  Thread count: 2"
    echo ""
    
    ./test_vggraph_opt.out test_dataset.txt test_queries.txt 0.5 3 2
    
    if [ $? -eq 0 ]; then
        echo ""
        echo "Test completed successfully!"
    else
        echo ""
        echo "Test failed during execution!"
        exit 1
    fi
else
    echo "Build failed!"
    exit 1
fi

echo ""
echo "VGGraph_Opt test completed."
