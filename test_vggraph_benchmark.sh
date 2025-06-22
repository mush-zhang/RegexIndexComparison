#!/bin/bash
# Test script for VGGraph_Opt benchmark integration

echo "Testing VGGraph_Opt Benchmark Integration"
echo "=========================================="

# Build the benchmark executable
echo "Building benchmark executable..."
cd /Users/lzhang55/Documents/GitHub/RegexIndexComparison

# Clean and rebuild
make clean
make benchmark.out

if [ $? -ne 0 ]; then
    echo "Build failed! Please check compilation errors."
    exit 1
fi

echo "Build successful!"

# Create test output directory
TEST_OUTPUT_DIR="test_vggraph_output"
mkdir -p $TEST_OUTPUT_DIR

# Run a small test with traffic data
echo ""
echo "Running VGGraph_Opt benchmark test..."
echo "Parameters:"
echo "  Method: VGGRAPH"
echo "  Threads: 2"
echo "  Workload: 1 (Traffic)"
echo "  Upper N: 3"
echo "  Selectivity: 0.2"
echo "  Experiments: 2"

./benchmark.out VGGRAPH -t 2 -w 1 -n 3 -c 0.2 -e 2 -o $TEST_OUTPUT_DIR

if [ $? -eq 0 ]; then
    echo ""
    echo "Benchmark test completed successfully!"
    echo ""
    echo "Output files generated:"
    ls -la $TEST_OUTPUT_DIR/
    echo ""
    echo "Summary file contents:"
    if [ -f "$TEST_OUTPUT_DIR/summary.csv" ]; then
        head -n 5 $TEST_OUTPUT_DIR/summary.csv
    fi
    echo ""
    echo "Stats file (if exists):"
    ls -la $TEST_OUTPUT_DIR/*stats.csv 2>/dev/null || echo "No stats file generated"
else
    echo ""
    echo "Benchmark test failed!"
    echo "Check the output above for error messages"
    exit 1
fi

echo ""
echo "VGGraph_Opt benchmark integration test completed."
