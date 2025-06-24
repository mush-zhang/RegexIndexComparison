#!/bin/bash

# Script to analyze regex literals for standard workloads using simple analyzer
# Usage: ./analyze_workloads_simple.sh [output_dir]

OUTPUT_DIR=${1:-"literal_analysis_results"}
mkdir -p "$OUTPUT_DIR"

echo "Building simple regex literal analyzer..."
make analyze_regex_literals_simple.out

if [ $? -ne 0 ]; then
    echo "Error: Failed to build analyzer"
    exit 1
fi

echo "Starting literal analysis for standard workloads..."
echo "Results will be saved to: $OUTPUT_DIR"
echo

# Define the standard regex files
declare -A workload_files=(
    ["traffic"]="data/regexes_traffic.txt"
    ["enron"]="data/enron/regexes_enron_wlit.txt"
    ["webpages"]="data/webpages/regexes_webpages.txt"
    ["protein"]="data/protein/prosites.txt"
    ["sysy"]="data/regexes_sysy.txt"
    ["dbx"]="data/regexes_dbx.txt"
)

# Check which files exist and analyze them
for workload in "${!workload_files[@]}"; do
    file=${workload_files[$workload]}
    
    echo "========================================"
    echo "Analyzing $workload workload"
    echo "File: $file"
    echo "========================================"
    
    if [ -f "$file" ]; then
        output_file="$OUTPUT_DIR/${workload}_literals.csv"
        
        ./analyze_regex_literals_simple.out -f "$file" -o "$output_file" -v
        
        if [ $? -eq 0 ]; then
            echo "✓ Analysis completed for $workload"
        else
            echo "✗ Analysis failed for $workload"
        fi
    else
        echo "✗ File not found: $file"
        echo "  Skipping $workload workload"
    fi
    echo
done

# Look for any other regex files in the data directory
echo "========================================"
echo "Searching for additional regex files..."
echo "========================================"

additional_files=$(find data -name "*.txt" -type f | grep -i regex | head -10)

if [ -n "$additional_files" ]; then
    echo "Found additional regex files:"
    echo "$additional_files"
    echo
    
    while IFS= read -r file; do
        if [[ ! " ${workload_files[@]} " =~ " $file " ]]; then
            basename=$(basename "$file" .txt)
            echo "Analyzing additional file: $file"
            output_file="$OUTPUT_DIR/${basename}_literals.csv"
            
            ./analyze_regex_literals_simple.out -f "$file" -o "$output_file"
            
            if [ $? -eq 0 ]; then
                echo "✓ Analysis completed for $basename"
            else
                echo "✗ Analysis failed for $basename"
            fi
            echo
        fi
    done <<< "$additional_files"
else
    echo "No additional regex files found."
fi

echo "========================================"
echo "Analysis completed!"
echo "Results directory: $OUTPUT_DIR"
echo "Files generated:"
ls -la "$OUTPUT_DIR" 2>/dev/null || echo "No files generated"
echo "========================================"
