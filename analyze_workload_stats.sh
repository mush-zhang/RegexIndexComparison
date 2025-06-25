#!/bin/bash

# Script to analyze dataset statistics for various workloads
# Usage: ./analyze_workload_stats.sh [output_dir]

OUTPUT_DIR=${1:-"dataset_stats_results"}
mkdir -p "$OUTPUT_DIR"

echo "Building dataset statistics analyzer..."
make analyze_dataset_stats.out

if [ $? -ne 0 ]; then
    echo "Error: Failed to build analyzer"
    exit 1
fi

echo "Starting dataset analysis..."
echo "Results will be saved to: $OUTPUT_DIR"
echo

# Define workload data paths
declare -A workload_paths=(
    ["enron"]="data/enron/maildir"
    ["webpages"]="data/webpages/processed"
    ["protein"]="data/protein/sequences"
    ["extracted"]="data/extracted"
)

# Define single file datasets
declare -A single_file_datasets=(
    ["traffic"]="data/US_Accidents_Dec21_updated.csv"
)

# Analyze directory-based datasets
for workload in "${!workload_paths[@]}"; do
    path=${workload_paths[$workload]}
    
    echo "========================================"
    echo "Analyzing $workload dataset"
    echo "Path: $path"
    echo "========================================"
    
    if [ -d "$path" ]; then
        output_file="$OUTPUT_DIR/${workload}_stats.csv"

        ./analyze_dataset_stats.out -d "$path" -n "$workload" -o "$output_file"

        if [ $? -eq 0 ]; then
            echo "✓ Analysis completed for $workload"
        else
            echo "✗ Analysis failed for $workload"
        fi
    else
        echo "✗ Directory not found: $path"
        echo "  Skipping $workload dataset"
    fi
    echo
done

# Analyze single file datasets
for workload in "${!single_file_datasets[@]}"; do
    file=${single_file_datasets[$workload]}
    
    echo "========================================"
    echo "Analyzing $workload dataset"
    echo "File: $file"
    echo "========================================"
    
    if [ -f "$file" ]; then
        output_file="$OUTPUT_DIR/${workload}_stats.csv"

        ./analyze_dataset_stats.out -f "$file" -n "$workload" -o "$output_file"

        if [ $? -eq 0 ]; then
            echo "✓ Analysis completed for $workload"
        else
            echo "✗ Analysis failed for $workload"
        fi
    else
        echo "✗ File not found: $file"
        echo "  Skipping $workload dataset"
    fi
    echo
done

# Create a combined summary
echo "========================================"
echo "Creating combined summary..."
echo "========================================"

summary_file="$OUTPUT_DIR/all_workloads_summary.csv"
echo "workload,total_lines,total_characters,alphabet_size,avg_line_length,min_line_length,max_line_length,median_line_length" > "$summary_file"

# Combine all individual CSV files
for csv_file in "$OUTPUT_DIR"/*_stats.csv; do
    if [ -f "$csv_file" ]; then
        # Skip header and append to summary
        tail -n +2 "$csv_file" >> "$summary_file"
    fi
done

echo "Combined summary saved to: $summary_file"

# Display summary table
if [ -f "$summary_file" ]; then
    echo
    echo "SUMMARY OF ALL WORKLOADS:"
    echo "========================="
    column -t -s, "$summary_file"
fi

echo
echo "========================================"
echo "Analysis completed!"
echo "Results directory: $OUTPUT_DIR"
echo "Files generated:"
ls -la "$OUTPUT_DIR" 2>/dev/null || echo "No files generated"
echo "========================================"
