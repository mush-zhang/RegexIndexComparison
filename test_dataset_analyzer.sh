#!/bin/bash

# Test script for dataset statistics analyzer
echo "Testing dataset statistics analyzer..."

# Build the analyzer
echo "Building analyzer..."
make analyze_dataset_stats.out

if [ $? -ne 0 ]; then
    echo "Error: Failed to build analyzer"
    exit 1
fi

echo "Build successful!"

# Create a test data directory with sample files
test_dir="test_dataset"
mkdir -p "$test_dir"

# Create sample data files
cat > "$test_dir/file1.txt" << 'EOF'
Hello world, this is a test file.
It contains multiple lines of text.
Each line has different lengths and characters.
Some lines are short.
Others are much longer and contain more diverse characters like numbers 123 and symbols @#$%.
EOF

cat > "$test_dir/file2.txt" << 'EOF'
Another test file with different content.
Email addresses: user@example.com, test@domain.org
Phone numbers: (555) 123-4567, 800-GET-HELP
Special characters: !@#$%^&*()_+-=[]{}|;:'",./<>?
Mixed case: ABC def GHI jkl MNO pqr STU vwx YZ
EOF

echo "Created test dataset in: $test_dir"
echo "Sample files:"
ls -la "$test_dir"
echo

# Test the analyzer on the directory
echo "Running analyzer on test directory..."
./analyze_dataset_stats.out -d "$test_dir" -n "test_dataset" -o "test_stats.csv"

# Check if it ran successfully
if [ $? -eq 0 ]; then
    echo
    echo "✓ Test completed successfully!"
    
    if [ -f "test_stats.csv" ]; then
        echo
        echo "Generated CSV content:"
        cat "test_stats.csv"
    fi
else
    echo "✗ Test failed"
    exit 1
fi

# Test on a single file
echo
echo "Testing on single file..."
./analyze_dataset_stats.out -f "$test_dir/file1.txt" -n "single_file_test"

# Check specific path for Enron (if it exists)
if [ -d "data/enron/maildir" ]; then
    echo
    echo "Testing on Enron dataset (limited to 100 lines)..."
    ./analyze_dataset_stats.out -d "data/enron/maildir" -n "enron_sample" -m 100 -o "enron_sample_stats.csv"
    
    if [ $? -eq 0 ] && [ -f "enron_sample_stats.csv" ]; then
        echo "Enron sample stats:"
        cat "enron_sample_stats.csv"
    fi
else
    echo
    echo "Enron dataset not found at data/enron/maildir - skipping that test"
fi

# Cleanup
rm -rf "$test_dir" "test_stats.csv" "enron_sample_stats.csv" 2>/dev/null
echo
echo "Test cleanup completed."
