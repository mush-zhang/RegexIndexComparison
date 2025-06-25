#!/bin/bash

# Comprehensive benchmark script for synthetic datasets
# Uses individual run scripts for each method (BEST, FREE, LPMS, TRIGRAM, VGGRAPH, BASELINE)

set -e  # Exit on any error

# Configuration
SYNTHETIC_DATA_DIR="./configurable_synthetic"
RESULTS_BASE_DIR="./result/synthetic_large_scale"
METADATA_FILE="${SYNTHETIC_DATA_DIR}/benchmark_metadata.pkl"

# Key upper bound configurations (following temp_vggraph_greedy_expr.sh pattern)
KEY_UPPER_BOUNDS=(50 100 200 500 1000 2000)

# Color codes for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

print_status() {
    echo -e "${BLUE}[$(date '+%Y-%m-%d %H:%M:%S')] $1${NC}"
}

print_success() {
    echo -e "${GREEN}[$(date '+%Y-%m-%d %H:%M:%S')] $1${NC}"
}

print_warning() {
    echo -e "${YELLOW}[$(date '+%Y-%m-%d %H:%M:%S')] $1${NC}"
}

print_error() {
    echo -e "${RED}[$(date '+%Y-%m-%d %H:%M:%S')] $1${NC}"
}

# Function to check if required files exist
check_prerequisites() {
    print_status "Checking prerequisites..."
    
    # Check individual run scripts
    local scripts=("run_expr_best.sh" "run_expr_free.sh" "run_expr_lpms.sh" 
                   "run_expr_trigram.sh" "run_expr_vggraph_greedy.sh" "run_expr_baseline.sh")
    
    for script in "${scripts[@]}"; do
        if [ ! -f "./${script}" ]; then
            print_error "Run script not found: ${script}"
            exit 1
        fi
        if [ ! -x "./${script}" ]; then
            print_warning "Making ${script} executable..."
            chmod +x "./${script}"
        fi
    done
    
    if [ ! -d "${SYNTHETIC_DATA_DIR}" ]; then
        print_error "Synthetic data directory not found: ${SYNTHETIC_DATA_DIR}"
        print_status "Please run './generate_synthetic_data.py' to generate synthetic data"
        exit 1
    fi
    
    if [ ! -f "${METADATA_FILE}" ]; then
        print_error "Metadata file not found: ${METADATA_FILE}"
        print_status "Please ensure synthetic data generation completed successfully"
        exit 1
    fi
    
    print_success "Prerequisites check passed"
}

# Function to extract dataset configurations from metadata
extract_configurations() {
    print_status "Extracting dataset configurations..."
    
    python3 << 'EOF'
import pickle
import sys

try:
    with open('./configurable_synthetic/benchmark_metadata.pkl', 'rb') as f:
        results = pickle.load(f)
    
    configs = []
    for result in results:
        config = {
            'identifier': result['identifier'],
            'dataset_file': result['dataset_file'],
            'query_file': result['query_file'],
            'alphabet_size': result['alphabet_size'],
            'dataset_size': result['dataset_size'],
            'query_set_size': result['query_set_size'],
            'target_selectivity': result['target_selectivity']
        }
        configs.append(config)
    
    # Write to temporary file for bash to read
    with open('./temp_configs.txt', 'w') as f:
        for config in configs:
            f.write(f"{config['identifier']}|{config['dataset_file']}|{config['query_file']}|{config['alphabet_size']}|{config['dataset_size']}|{config['query_set_size']}|{config['target_selectivity']}\n")
    
    print(f"Found {len(configs)} dataset configurations")
    
except Exception as e:
    print(f"Error reading metadata: {e}")
    sys.exit(1)
EOF
    
    if [ $? -ne 0 ]; then
        print_error "Failed to extract configurations"
        exit 1
    fi
    
    print_success "Extracted configurations successfully"
}

# Function to run BEST benchmarks
run_best_benchmarks() {
    print_status "Starting BEST benchmarks..."
    
    local success_count=0
    local total_count=0
    
    while IFS='|' read -r identifier dataset_file query_file alphabet_size dataset_size query_set_size target_selectivity; do
        # Run without key upper bound
        total_count=$((total_count + 1))
        local workload_name="synthetic_${identifier}"
        
        print_status "Running BEST on ${identifier} (no key limit)"
        if ./run_expr_best.sh -w "${workload_name}" -r "${query_file}" -d "${dataset_file}"; then
            success_count=$((success_count + 1))
            print_success "BEST completed: ${identifier}"
        else
            print_error "BEST failed: ${identifier}"
        fi
        
        # Run with different key upper bounds
        for k in "${KEY_UPPER_BOUNDS[@]}"; do
            total_count=$((total_count + 1))
            print_status "Running BEST on ${identifier} (key limit: ${k})"
            if ./run_expr_best.sh -w "${workload_name}_k${k}" -r "${query_file}" -d "${dataset_file}" -k "${k}"; then
                success_count=$((success_count + 1))
                print_success "BEST completed: ${identifier} (k=${k})"
            else
                print_error "BEST failed: ${identifier} (k=${k})"
            fi
        done
    done < temp_configs.txt
    
    print_status "BEST benchmarks completed: ${success_count}/${total_count} successful"
}

# Function to run FREE benchmarks
run_free_benchmarks() {
    print_status "Starting FREE benchmarks..."
    
    local success_count=0
    local total_count=0
    
    while IFS='|' read -r identifier dataset_file query_file alphabet_size dataset_size query_set_size target_selectivity; do
        # Run without key upper bound
        total_count=$((total_count + 1))
        local workload_name="synthetic_${identifier}"
        
        print_status "Running FREE on ${identifier} (no key limit)"
        if ./run_expr_free.sh -w "${workload_name}" -r "${query_file}" -d "${dataset_file}"; then
            success_count=$((success_count + 1))
            print_success "FREE completed: ${identifier}"
        else
            print_error "FREE failed: ${identifier}"
        fi
        
        # Run with different key upper bounds
        for k in "${KEY_UPPER_BOUNDS[@]}"; do
            total_count=$((total_count + 1))
            print_status "Running FREE on ${identifier} (key limit: ${k})"
            if ./run_expr_free.sh -w "${workload_name}_k${k}" -r "${query_file}" -d "${dataset_file}" -k "${k}"; then
                success_count=$((success_count + 1))
                print_success "FREE completed: ${identifier} (k=${k})"
            else
                print_error "FREE failed: ${identifier} (k=${k})"
            fi
        done
    done < temp_configs.txt
    
    print_status "FREE benchmarks completed: ${success_count}/${total_count} successful"
}

# Function to run LPMS benchmarks
run_lpms_benchmarks() {
    print_status "Starting LPMS benchmarks..."
    
    local success_count=0
    local total_count=0
    
    while IFS='|' read -r identifier dataset_file query_file alphabet_size dataset_size query_set_size target_selectivity; do
        # Run without key upper bound
        total_count=$((total_count + 1))
        local workload_name="synthetic_${identifier}"
        
        print_status "Running LPMS on ${identifier} (no key limit)"
        if ./run_expr_lpms.sh -w "${workload_name}" -r "${query_file}" -d "${dataset_file}"; then
            success_count=$((success_count + 1))
            print_success "LPMS completed: ${identifier}"
        else
            print_error "LPMS failed: ${identifier}"
        fi
        
        # Run with different key upper bounds
        for k in "${KEY_UPPER_BOUNDS[@]}"; do
            total_count=$((total_count + 1))
            print_status "Running LPMS on ${identifier} (key limit: ${k})"
            if ./run_expr_lpms.sh -w "${workload_name}_k${k}" -r "${query_file}" -d "${dataset_file}" -k "${k}"; then
                success_count=$((success_count + 1))
                print_success "LPMS completed: ${identifier} (k=${k})"
            else
                print_error "LPMS failed: ${identifier} (k=${k})"
            fi
        done
    done < temp_configs.txt
    
    print_status "LPMS benchmarks completed: ${success_count}/${total_count} successful"
}

# Function to run TRIGRAM benchmarks
run_trigram_benchmarks() {
    print_status "Starting TRIGRAM benchmarks..."
    
    local success_count=0
    local total_count=0
    
    while IFS='|' read -r identifier dataset_file query_file alphabet_size dataset_size query_set_size target_selectivity; do
        # Run without key upper bound
        total_count=$((total_count + 1))
        local workload_name="synthetic_${identifier}"
        
        print_status "Running TRIGRAM on ${identifier} (no key limit)"
        if ./run_expr_trigram.sh -w "${workload_name}" -r "${query_file}" -d "${dataset_file}"; then
            success_count=$((success_count + 1))
            print_success "TRIGRAM completed: ${identifier}"
        else
            print_error "TRIGRAM failed: ${identifier}"
        fi
        
        # Run with different key upper bounds
        for k in "${KEY_UPPER_BOUNDS[@]}"; do
            total_count=$((total_count + 1))
            print_status "Running TRIGRAM on ${identifier} (key limit: ${k})"
            if ./run_expr_trigram.sh -w "${workload_name}_k${k}" -r "${query_file}" -d "${dataset_file}" -k "${k}"; then
                success_count=$((success_count + 1))
                print_success "TRIGRAM completed: ${identifier} (k=${k})"
            else
                print_error "TRIGRAM failed: ${identifier} (k=${k})"
            fi
        done
    done < temp_configs.txt
    
    print_status "TRIGRAM benchmarks completed: ${success_count}/${total_count} successful"
}

# Function to run VGGRAPH benchmarks
run_vggraph_benchmarks() {
    print_status "Starting VGGRAPH benchmarks..."
    
    local success_count=0
    local total_count=0
    
    while IFS='|' read -r identifier dataset_file query_file alphabet_size dataset_size query_set_size target_selectivity; do
        # Run without key upper bound
        total_count=$((total_count + 1))
        local workload_name="synthetic_${identifier}"
        
        print_status "Running VGGRAPH on ${identifier} (no key limit)"
        if ./run_expr_vggraph_greedy.sh -w "${workload_name}" -r "${query_file}" -d "${dataset_file}"; then
            success_count=$((success_count + 1))
            print_success "VGGRAPH completed: ${identifier}"
        else
            print_error "VGGRAPH failed: ${identifier}"
        fi
        
        # Run with different key upper bounds
        for k in "${KEY_UPPER_BOUNDS[@]}"; do
            total_count=$((total_count + 1))
            print_status "Running VGGRAPH on ${identifier} (key limit: ${k})"
            if ./run_expr_vggraph_greedy.sh -w "${workload_name}_k${k}" -r "${query_file}" -d "${dataset_file}" -k "${k}"; then
                success_count=$((success_count + 1))
                print_success "VGGRAPH completed: ${identifier} (k=${k})"
            else
                print_error "VGGRAPH failed: ${identifier} (k=${k})"
            fi
        done
    done < temp_configs.txt
    
    print_status "VGGRAPH benchmarks completed: ${success_count}/${total_count} successful"
}

# Function to run BASELINE benchmarks
run_baseline_benchmarks() {
    print_status "Starting BASELINE benchmarks..."
    
    local success_count=0
    local total_count=0
    
    while IFS='|' read -r identifier dataset_file query_file alphabet_size dataset_size query_set_size target_selectivity; do
        total_count=$((total_count + 1))
        local workload_name="synthetic_${identifier}"
        
        print_status "Running BASELINE on ${identifier}"
        if ./run_expr_baseline.sh -w "${workload_name}" -r "${query_file}" -d "${dataset_file}"; then
            success_count=$((success_count + 1))
            print_success "BASELINE completed: ${identifier}"
        else
            print_error "BASELINE failed: ${identifier}"
        fi
    done < temp_configs.txt
    
    print_status "BASELINE benchmarks completed: ${success_count}/${total_count} successful"
}

# Function to generate summary report
generate_summary() {
    print_status "Generating summary report..."
    
    local summary_file="${RESULTS_BASE_DIR}/benchmark_summary.txt"
    mkdir -p "${RESULTS_BASE_DIR}"
    
    {
        echo "Synthetic Dataset Benchmark Summary"
        echo "=================================="
        echo "Generated on: $(date)"
        echo ""
        echo "Configuration:"
        echo "- Data directory: ${SYNTHETIC_DATA_DIR}"
        echo "- Results directory: ${RESULTS_BASE_DIR}"
        echo "- Key upper bounds tested: ${KEY_UPPER_BOUNDS[*]}"
        echo ""
        echo "Methods tested:"
        echo "- BEST (with and without key limits)"
        echo "- FREE (with and without key limits)"
        echo "- LPMS (with and without key limits)"
        echo "- TRIGRAM (with and without key limits)"
        echo "- VGGRAPH (with and without key limits)"
        echo "- BASELINE (no key limits applicable)"
        echo ""
        echo "Result directories:"
        find "./result" -type d -name "*synthetic*" 2>/dev/null | sort || echo "No result directories found yet"
    } > "${summary_file}"
    
    print_success "Summary report generated: ${summary_file}"
}

# Function to cleanup temporary files
cleanup() {
    print_status "Cleaning up temporary files..."
    rm -f temp_configs.txt
    print_success "Cleanup completed"
}

# Main execution function
main() {
    print_status "Starting comprehensive synthetic dataset benchmarks"
    print_status "Using individual run scripts for each method"
    
    # Setup
    check_prerequisites
    extract_configurations
    
    # Create results directory
    mkdir -p "${RESULTS_BASE_DIR}"
    
    # Run benchmarks for each method
    local methods_to_run=()
    
    # Parse command line arguments for specific methods
    if [ $# -eq 0 ]; then
        # Run all methods by default
        methods_to_run=("BEST" "FREE" "LPMS" "TRIGRAM" "VGGRAPH" "BASELINE")
    else
        # Run only specified methods
        methods_to_run=("$@")
    fi
    
    for method in "${methods_to_run[@]}"; do
        case "${method^^}" in
            "BEST")
                run_best_benchmarks
                ;;
            "FREE")
                run_free_benchmarks
                ;;
            "LPMS")
                run_lpms_benchmarks
                ;;
            "TRIGRAM")
                run_trigram_benchmarks
                ;;
            "VGGRAPH")
                run_vggraph_benchmarks
                ;;
            "BASELINE")
                run_baseline_benchmarks
                ;;
            *)
                print_warning "Unknown method: ${method}"
                ;;
        esac
    done
    
    # Generate summary
    generate_summary
    
    # Cleanup
    cleanup
    
    print_success "All benchmarks completed successfully!"
    print_status "Results available in ./result/ directory"
}

# Handle script interruption
trap cleanup EXIT

# Show usage if help requested
if [[ "$1" == "-h" || "$1" == "--help" ]]; then
    echo "Usage: $0 [METHOD1] [METHOD2] ..."
    echo ""
    echo "Run benchmarks on synthetic datasets using individual run scripts"
    echo ""
    echo "Available methods:"
    echo "  BEST      - BEST n-gram selection method"
    echo "  FREE      - FREE multi-gram selection method"
    echo "  LPMS      - LPMS (Fast) method with deterministic relaxation"
    echo "  TRIGRAM   - Traditional trigram indexing"
    echo "  VGGRAPH   - VGGraph Greedy n-gram selection"
    echo "  BASELINE  - No indexing (baseline)"
    echo ""
    echo "Each method (except BASELINE) will be run with:"
    echo "  1. No key upper bound limit"
    echo "  2. Key upper bounds: ${KEY_UPPER_BOUNDS[*]}"
    echo ""
    echo "If no methods specified, all methods will be run."
    echo ""
    echo "Example:"
    echo "  $0                    # Run all methods"
    echo "  $0 BEST FREE          # Run only BEST and FREE"
    echo "  $0 VGGRAPH            # Run only VGGRAPH"
    echo ""
    echo "Prerequisites:"
    echo "  - Run './generate_synthetic_data.py' first to generate datasets"
    echo "  - Ensure all run_expr_*.sh scripts are present and executable"
    exit 0
fi

# Run main function
main "$@"
