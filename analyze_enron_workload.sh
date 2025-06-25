#!/bin/bash

# Script to analyze Enron dataset and workload statistics
# Focuses on alphabet size, average line length, and regex literal analysis

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
DATA_DIR="$SCRIPT_DIR/data"
ENRON_DIR="$DATA_DIR/enron"
OUTPUT_DIR="$SCRIPT_DIR/analysis_results"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}=== Enron Dataset and Workload Analysis ===${NC}"
echo "Analysis Directory: $OUTPUT_DIR"

# Create output directory
mkdir -p "$OUTPUT_DIR"

# Check if required tools are built
echo -e "${BLUE}Checking required tools...${NC}"

if [ ! -f "analyze_dataset_stats.out" ]; then
    echo -e "${YELLOW}Building dataset statistics analyzer...${NC}"
    make analyze_dataset_stats.out
fi

if [ ! -f "analyze_regex_literals_simple.out" ]; then
    echo -e "${YELLOW}Building regex literal analyzer...${NC}"
    make analyze_regex_literals_simple.out
fi

echo -e "${GREEN}✓ All tools ready${NC}"
echo

# Function to analyze regex workload files
analyze_regex_workload() {
    local workload_file="$1"
    local workload_name="$2"
    
    if [ -f "$workload_file" ]; then
        echo -e "${BLUE}Analyzing $workload_name regex workload...${NC}"
        echo "File: $workload_file"
        
        # Count total regexes
        local total_regexes=$(wc -l < "$workload_file" 2>/dev/null || echo "0")
        echo "Total regexes: $total_regexes"
        
        # Analyze regex literals
        local output_file="$OUTPUT_DIR/${workload_name}_literal_analysis.csv"
        echo "Running literal analysis..."
        ./analyze_regex_literals_simple.out -f "$workload_file" -o "$output_file" # -v
        if [ -f "$output_file" ]; then
            echo -e "${GREEN}✓ Literal analysis saved to: $output_file${NC}"
            
            # Show summary statistics
            echo "Summary from literal analysis:"
            tail -n 5 "$output_file" | head -n 3
        else
            echo -e "${RED}✗ Failed to generate literal analysis${NC}"
        fi
        
        echo
    else
        echo -e "${YELLOW}⚠ Workload file not found: $workload_file${NC}"
        echo
    fi
}

# Function to analyze dataset directory
analyze_dataset_directory() {
    local dataset_dir="$1"
    local dataset_name="$2"
    
    if [ -d "$dataset_dir" ]; then
        echo -e "${BLUE}Analyzing $dataset_name dataset directory...${NC}"
        echo "Directory: $dataset_dir"
        
        # Count files
        local file_count=$(find "$dataset_dir" -type f 2>/dev/null | wc -l || echo "0")
        echo "Total files: $file_count"
        
        if [ "$file_count" -gt 0 ]; then
            # Analyze dataset statistics
            local output_file="$OUTPUT_DIR/${dataset_name}_dataset_stats.csv"
            echo "Running dataset statistics analysis..."
            ./analyze_dataset_stats.out "$dataset_dir" > "$output_file"
            
            if [ -f "$output_file" ]; then
                echo -e "${GREEN}✓ Dataset analysis saved to: $output_file${NC}"
                
                # Show key statistics
                echo "Key statistics:"
                grep -E "(alphabet_size|avg_line_length|total_lines)" "$output_file" | head -n 3
            else
                echo -e "${RED}✗ Failed to generate dataset analysis${NC}"
            fi
        else
            echo -e "${YELLOW}⚠ No files found in directory${NC}"
        fi
        
        echo
    else
        echo -e "${YELLOW}⚠ Dataset directory not found: $dataset_dir${NC}"
        echo
    fi
}

# Analyze Enron workload files
echo -e "${BLUE}=== ENRON WORKLOAD ANALYSIS ===${NC}"
analyze_regex_workload "$ENRON_DIR/regexes_enron.txt" "enron"
analyze_regex_workload "$ENRON_DIR/regexes_enron_wlit.txt" "enron_wlit"
analyze_regex_workload "$ENRON_DIR/regexes_enron_test.txt" "enron_test"

# Check for Enron maildir dataset
echo -e "${BLUE}=== ENRON DATASET ANALYSIS ===${NC}"
ENRON_MAILDIR="$ENRON_DIR/maildir"
if [ -d "$ENRON_MAILDIR" ]; then
    analyze_dataset_directory "$ENRON_MAILDIR" "enron_maildir"
else
    echo -e "${YELLOW}⚠ Enron maildir dataset not found at: $ENRON_MAILDIR${NC}"
    echo "The Enron email dataset needs to be downloaded separately."
    echo "You can download it from: https://www.cs.cmu.edu/~enron/"
    echo
fi

# Analyze other available datasets
echo -e "${BLUE}=== OTHER DATASET ANALYSIS ===${NC}"

# Check for other regex workloads
analyze_regex_workload "$DATA_DIR/regexes_traffic.txt" "traffic"
analyze_regex_workload "$DATA_DIR/webpages/regexes_webpages.txt" "webpages"
analyze_regex_workload "$DATA_DIR/webpages/regexes_webpages_free.txt" "webpages_free"

# Summary
echo -e "${BLUE}=== ANALYSIS SUMMARY ===${NC}"
echo "Analysis complete! Results saved in: $OUTPUT_DIR"
echo
echo "Generated files:"
ls -la "$OUTPUT_DIR" 2>/dev/null || echo "No output files generated"

echo
echo -e "${GREEN}=== QUICK ALPHABET SIZE SUMMARY ===${NC}"
echo "Alphabet sizes from available datasets:"

# Extract alphabet sizes from generated CSV files
for file in "$OUTPUT_DIR"/*_dataset_stats.csv; do
    if [ -f "$file" ]; then
        dataset_name=$(basename "$file" _dataset_stats.csv)
        alphabet_size=$(grep "alphabet_size" "$file" 2>/dev/null | cut -d',' -f2 || echo "N/A")
        avg_length=$(grep "avg_line_length" "$file" 2>/dev/null | cut -d',' -f2 || echo "N/A")
        echo "  $dataset_name: Alphabet size = $alphabet_size, Avg line length = $avg_length"
    fi
done

echo
echo -e "${GREEN}Analysis complete!${NC}"
echo "To visualize results, you can use: python3 analyze_results.py $OUTPUT_DIR"
