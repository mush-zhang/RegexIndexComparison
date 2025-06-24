#!/bin/bash

# Generate large-scale synthetic datasets using the preprocess_synthetic.ipynb notebook

set -e

NOTEBOOK_PATH="notebooks/preprocess_synthetic.ipynb"
OUTPUT_DIR="./configurable_synthetic"

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

print_error() {
    echo -e "${RED}[$(date '+%Y-%m-%d %H:%M:%S')] $1${NC}"
}

check_prerequisites() {
    print_status "Checking prerequisites..."
    
    if [ ! -f "${NOTEBOOK_PATH}" ]; then
        print_error "Notebook not found: ${NOTEBOOK_PATH}"
        exit 1
    fi
    
    # Check if jupyter is available
    if ! command -v jupyter &> /dev/null; then
        print_error "Jupyter not found. Please install jupyter notebook."
        exit 1
    fi
    
    print_success "Prerequisites check passed"
}

generate_synthetic_data() {
    print_status "Generating large-scale synthetic datasets..."
    
    # Create a temporary Python script to execute the notebook cells
    cat > temp_generate_synthetic.py << 'EOF'
import subprocess
import sys
import os

# Execute the notebook to generate synthetic data
def run_notebook():
    try:
        # Execute the notebook with the large-scale configuration
        result = subprocess.run([
            'jupyter', 'nbconvert', '--to', 'python', '--execute', 
            'notebooks/preprocess_synthetic.ipynb', '--stdout'
        ], capture_output=True, text=True, timeout=3600)  # 1 hour timeout
        
        if result.returncode != 0:
            print(f"Error executing notebook: {result.stderr}")
            return False
        
        print("Notebook executed successfully")
        return True
        
    except subprocess.TimeoutExpired:
        print("Notebook execution timed out")
        return False
    except Exception as e:
        print(f"Error: {e}")
        return False

def run_direct_generation():
    """Run the synthetic data generation directly using Python."""
    import sys
    sys.path.append('.')
    
    try:
        # Import the notebook as a module and run the generation
        import importlib.util
        
        # Convert notebook to python temporarily
        subprocess.run([
            'jupyter', 'nbconvert', '--to', 'python', 
            'notebooks/preprocess_synthetic.ipynb', '--output', 'temp_synthetic_gen'
        ], check=True)
        
        # Load and execute the generated python module
        spec = importlib.util.spec_from_file_location("temp_synthetic_gen", "temp_synthetic_gen.py")
        module = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(module)
        
        # Run the large-scale generation
        print("Running large-scale synthetic data generation...")
        results = module.generate_large_scale_benchmarks()
        print(f"Generated {len(results)} configurations")
        
        # Clean up
        os.remove("temp_synthetic_gen.py")
        
        return True
        
    except Exception as e:
        print(f"Error in direct generation: {e}")
        return False

if __name__ == "__main__":
    success = run_direct_generation()
    if not success:
        print("Trying notebook execution method...")
        success = run_notebook()
    
    sys.exit(0 if success else 1)
EOF
    
    # Run the generation
    if python3 temp_generate_synthetic.py; then
        print_success "Synthetic data generation completed"
    else
        print_error "Failed to generate synthetic data"
        rm -f temp_generate_synthetic.py
        exit 1
    fi
    
    # Clean up
    rm -f temp_generate_synthetic.py
}

verify_output() {
    print_status "Verifying generated data..."
    
    if [ ! -d "${OUTPUT_DIR}" ]; then
        print_error "Output directory not found: ${OUTPUT_DIR}"
        return 1
    fi
    
    if [ ! -f "${OUTPUT_DIR}/benchmark_metadata.pkl" ]; then
        print_error "Metadata file not found: ${OUTPUT_DIR}/benchmark_metadata.pkl"
        return 1
    fi
    
    # Count generated files
    local dataset_count=$(find "${OUTPUT_DIR}" -name "dataset_*.txt" | wc -l)
    local query_count=$(find "${OUTPUT_DIR}" -name "queries_*.txt" | wc -l)
    
    print_status "Found ${dataset_count} dataset files and ${query_count} query files"
    
    if [ "${dataset_count}" -eq 0 ] || [ "${query_count}" -eq 0 ]; then
        print_error "No dataset or query files found"
        return 1
    fi
    
    print_success "Data verification passed"
    return 0
}

main() {
    print_status "Starting large-scale synthetic data generation"
    
    check_prerequisites
    
    # Check if data already exists
    if [ -d "${OUTPUT_DIR}" ] && [ -f "${OUTPUT_DIR}/benchmark_metadata.pkl" ]; then
        print_status "Synthetic data already exists in ${OUTPUT_DIR}"
        read -p "Regenerate data? (y/N): " -n 1 -r
        echo
        if [[ ! $REPLY =~ ^[Yy]$ ]]; then
            print_status "Using existing data"
            verify_output
            exit 0
        fi
        print_status "Removing existing data..."
        rm -rf "${OUTPUT_DIR}"
    fi
    
    generate_synthetic_data
    verify_output
    
    print_success "Large-scale synthetic data generation completed!"
    print_status "Data available in: ${OUTPUT_DIR}"
    print_status "You can now run: ./run_synthetic_benchmarks.sh"
}

# Show usage if help requested
if [[ "$1" == "-h" || "$1" == "--help" ]]; then
    echo "Usage: $0"
    echo ""
    echo "Generate large-scale synthetic datasets using the preprocess_synthetic.ipynb notebook"
    echo ""
    echo "This script will:"
    echo "1. Execute the notebook to generate synthetic data with large_scale_config"
    echo "2. Create datasets and queries in ${OUTPUT_DIR}"
    echo "3. Generate metadata for benchmarking"
    echo ""
    echo "Prerequisites:"
    echo "- Jupyter notebook installed"
    echo "- Python 3 with required packages (numpy, pandas, matplotlib, etc.)"
    exit 0
fi

main "$@"
