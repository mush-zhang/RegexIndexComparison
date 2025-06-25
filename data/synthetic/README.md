# Synthetic Workloads for Regex Index Comparison

This directory contains synthetic workload generation scripts and documentation for the RegexIndexComparison benchmark suite. The synthetic workloads are designed to test different aspects of regex indexing performance under controlled conditions.

## Overview

The synthetic workloads are generated using `preprocess_synthetic.py`, which creates multiple experimental scenarios (Expr 1-5) with varying characteristics:

- **Expr 1**: Trigram-based datasets with different frequency distributions
- **Expr 2**: Fixed-length strings with variable dataset sizes and query workloads
- **Expr 4**: Variable alphabet size datasets (Rob01-Rob04)
- **Expr 5**: Three-segment regex patterns with variable gaps

Additionally, the `generate_large_scale_synthetic.sh` script provides **configurable large-scale synthetic workloads** using the `ConfigurableSyntheticGenerator` class with multiple predefined configurations for comprehensive benchmarking.

## Large-Scale Configurable Workloads

The `generate_large_scale_synthetic.sh` script generates sophisticated synthetic workloads using the `ConfigurableSyntheticGenerator` from `notebooks/preprocess_synthetic.ipynb`. This provides more systematic and configurable workload generation with precise control over multiple parameters.

### Available Configurations

**Small Scale Config**:
- Alphabet sizes: [4, 8, 16] characters
- Dataset sizes: [1K, 5K, 10K] strings
- Query set sizes: [50, 100, 200] queries
- Selectivity targets: [0.01, 0.05, 0.1, 0.2]
- String lengths: Mean=100, Std=20, Range=[20, 200]

**Medium Scale Config**:
- Alphabet sizes: [8, 16, 26, 32] characters  
- Dataset sizes: [10K, 50K, 100K] strings
- Query set sizes: [100, 500, 1K] queries
- Selectivity targets: [0.005, 0.01, 0.02, 0.05, 0.1]
- String lengths: Mean=150, Std=30, Range=[50, 300]

**Large Scale Config**:
- Alphabet sizes: [16, 26, 52, 64] characters (uppercase, upper+lower, alphanumeric, extended)
- Dataset sizes: [50K, 100K, 200K, 500K] strings
- Query set sizes: [500, 1K, 2K, 5K] queries
- Selectivity targets: [0.001, 0.005, 0.01, 0.02, 0.05, 0.1, 0.2]
- String lengths: Mean=200, Std=50, Range=[50, 500]

**Fixed Length Config**:
- Alphabet sizes: [4, 8, 12, 16, 26] characters
- Dataset sizes: [20K, 40K, 60K, 80K, 100K] strings
- Query set sizes: [100, 500, 2K, 2.5K, 5K] queries  
- Fixed string length: 450 characters (similar to Expr 2)

### Generated Output Structure

The large-scale generator creates a comprehensive directory structure:

```
./configurable_synthetic/
├── benchmark_metadata.pkl          # Complete configuration metadata
├── dataset_*.txt                   # Generated datasets 
├── queries_*.txt                   # Generated query workloads
└── analysis_results/               # Selectivity analysis and statistics
    ├── selectivity_report.txt      # Detailed selectivity analysis
    └── config_summary.json         # Summary of all generated configurations
```

Each configuration generates:
- **Dataset file**: `dataset_{alphabet}_{size}_{config_id}.txt`
- **Query file**: `queries_{alphabet}_{size}_{queries}_{selectivity}_{config_id}.txt`
- **Metadata**: Complete parameter tracking and actual measured selectivity

### Key Features of Configurable Generator

**Precise Selectivity Control**:
- Target selectivity specification with validation
- Actual selectivity measurement and reporting
- Automatic query adjustment to meet targets

**Comprehensive Parameter Space**:
- Cartesian product of all configuration parameters
- Systematic coverage of experimental dimensions
- Reproducible random seed management

**Advanced Query Generation**:
- Intelligent literal extraction from dataset strings
- Gap size optimization for target selectivity
- Multi-segment pattern support (2-3 segments)

**Performance Optimization**:
- Efficient string generation algorithms
- Memory-conscious handling of large datasets
- Progress tracking and error reporting

**Usage**:
```bash
# Generate large-scale configurable workloads
./generate_large_scale_synthetic.sh

# Output will be in ./configurable_synthetic/
# Verify generation with analysis tools
python3 analyze_results.py ./configurable_synthetic/
```

## Experimental Scenarios

### Expr 1: Trigram Frequency Distribution Analysis

**Purpose**: Test how different trigram frequency distributions affect regex indexing performance.

**Characteristics**:
- Generates 4 different datasets with varying trigram frequency distributions
- Dataset size: 400,000 strings per dataset
- Uses normal distributions with different means and standard deviations
- Frequency parameters:
  - Dataset 1: Mean=(800, 600), StdDev=100
  - Dataset 2: Mean=(1200, 400), StdDev=200  
  - Dataset 3: Mean=(100, 1900), StdDev=300
  - Dataset 4: Mean=(0, 3700), StdDev=400
- Query counts: 227-248 queries per dataset (randomized)

**Generated Files**:
- `synthetic1/dataset_1.txt` - `synthetic1/dataset_4.txt`
- `synthetic1/queries_1.txt` - `synthetic1/queries_4.txt`
- Histogram plots showing trigram distribution analysis

**Key Features**:
- Trigram-based string generation using all possible 3-character combinations
- Variable-length strings with probabilistic termination
- Bimodal frequency distributions to simulate real-world data patterns

### Expr 2: Scalability Testing

**Purpose**: Evaluate performance scalability across different dataset sizes and query workload sizes.

**Characteristics**:
- Fixed string length: 450 characters
- Alphabet: English uppercase letters (A-Z)
- Dataset sizes: 20K, 40K, 60K, 80K, 100K strings
- Query workload sizes: 100, 500, 2K, 2.5K, 5K queries
- Query generation: Extract two literal segments with gaps from dataset strings

**Generated Files**:
- **Datasets**: `expr2/datasets/dataset_{size}.txt`
  - `dataset_20000.txt`, `dataset_40000.txt`, etc.
- **Variable workloads** (based on 20K dataset): `expr2/queries/query_workload_{size}.txt`
  - `query_workload_100.txt`, `query_workload_500.txt`, etc.
- **Fixed workloads** (1000 queries per dataset): `expr2/queries/query_workload_1000_for_{size}.txt`

**Query Pattern**:
- Format: `literal1(.{0,gap_size})literal2`
- Literal lengths: 3-8 characters
- Gap sizes: 1-50 characters
- Extracted from 10% sample of the source dataset

### Expr 4: Alphabet Size Variation (Rob Datasets)

**Purpose**: Study the impact of alphabet size on regex indexing performance.

**Characteristics**:
- Variable alphabet sizes:
  - **Rob01**: 4 characters (A-D)
  - **Rob02**: 8 characters (A-H)  
  - **Rob03**: 12 characters (A-L)
  - **Rob04**: 16 characters (A-P)
- 5,000 strings per dataset
- Variable string lengths with probabilistic termination (1/(10*alphabet_size) probability)
- Multiple query workload sizes: 10%, 30%, 50% of dataset size
- Test queries: 2% of dataset size

**Generated Files**:
- **Datasets**: `expr4/datasets/{Rob01,Rob02,Rob03,Rob04}.txt`
- **Query workloads**: `expr4/queries/{RobXX}_queries_{10,30,50}pct.txt`
- **Test queries**: `expr4/queries/{RobXX}_test_queries_2pct.txt`

**Query Pattern**:
- Same as Expr 2: `literal1(.{0,gap_size})literal2`
- Generated from samples of the corresponding dataset

### Expr 5: Complex Multi-Segment Patterns

**Purpose**: Test performance with more complex regex patterns containing multiple gaps.

**Characteristics**:
- Same alphabet variations as Expr 4 (Rob01-Rob04)
- 5,000 strings per dataset
- Three-segment query patterns with two gaps
- Multiple query workload sizes: 10%, 30%, 50% of dataset size

**Generated Files**:
- **Datasets**: `expr5/datasets/{Rob01,Rob02,Rob03,Rob04}.txt`
- **Query workloads**: `expr5/queries/{RobXX}_queries_{10,30,50}pct.txt`
- **Test queries**: `expr5/queries/{RobXX}_test_queries_2pct.txt`

**Query Pattern**:
- Format: `literal1(.{0,gap1})literal2(.{0,gap2})literal3`
- Three literal segments with two variable gaps
- More complex than Expr 2/4 patterns

## Generation Parameters

### Common Parameters
- **Random Seeds**: 
  - `random.seed(53711)`
  - `np.random.seed(15213)`
- **Literal Length Range**: 3-8 characters
- **Gap Size Range**: 1-50 characters
- **Sample Percentage**: 10% of dataset for query generation

### String Generation Methods
1. **Trigram-based** (Expr 1): Uses predefined trigram frequencies
2. **Random Choice** (Expr 2): Uniform random selection from alphabet
3. **Probabilistic Termination** (Expr 4, 5): Variable length with termination probability

## Usage

### Generate Large-Scale Configurable Workloads (Recommended)
```bash
# Generate comprehensive large-scale synthetic workloads
./generate_large_scale_synthetic.sh

# This will create ./configurable_synthetic/ with systematic parameter coverage
```

### Generate Traditional Experimental Workloads
```bash
cd data/synthetic
python3 preprocess_synthetic.py
```

### Generate Specific Experiments
The `preprocess_synthetic.py` script contains individual functions for each experiment:
- `generate_expr1()` - Trigram frequency analysis
- `generate_expr2()` - Scalability testing  
- `generate_expr4()` - Alphabet size variation
- `generate_expr5()` - Multi-segment patterns

Uncomment the relevant function calls at the end of the script to generate specific experiments.

### Choose Generation Method
- **Use `generate_large_scale_synthetic.sh`** for comprehensive benchmarking with systematic parameter coverage
- **Use `preprocess_synthetic.py`** for specific experimental scenarios or when reproducing exact legacy workloads

## Analysis Features

### Histogram Analysis (Expr 1)
- Generates distribution plots for trigram frequencies
- Saves plots as PDF files in `histogram_plots/`
- Shows how many strings contain each trigram

### Statistics Tracking
- Trigram frequency counters
- String length distributions
- Query pattern complexity metrics

## File Structure

### Traditional Experiments (preprocess_synthetic.py)
```
data/synthetic/
├── preprocess_synthetic.py          # Traditional generation script
├── expr2/                          # Scalability experiments
│   ├── datasets/                   # Variable size datasets
│   └── queries/                    # Variable and fixed workloads
├── expr4/                          # Alphabet size experiments  
│   ├── datasets/                   # Rob01-Rob04 datasets
│   └── queries/                    # Query workloads by percentage
├── expr5/                          # Multi-segment experiments
│   ├── datasets/                   # Rob01-Rob04 datasets
│   └── queries/                    # Complex pattern workloads
├── synthetic1/                     # Trigram frequency experiments
│   ├── dataset_*.txt              # Frequency-based datasets
│   └── queries_*.txt              # Corresponding queries
└── histogram_plots/                # Analysis visualizations
    └── dataset_*_histogram.pdf    # Distribution plots
```

### Large-Scale Configurable Workloads (generate_large_scale_synthetic.sh)
```
./configurable_synthetic/           # Generated by large-scale script
├── benchmark_metadata.pkl          # Complete configuration metadata
├── dataset_*.txt                   # Systematically generated datasets
├── queries_*.txt                   # Systematically generated queries
├── analysis_results/               # Detailed analysis and statistics
│   ├── selectivity_report.txt      # Selectivity analysis for all configs
│   ├── config_summary.json         # Summary of generated configurations
│   └── parameter_coverage.txt      # Parameter space coverage report
└── logs/                           # Generation logs and progress
    ├── generation_log.txt          # Detailed generation log
    └── error_report.txt            # Any errors or warnings
```

## Design Rationale

### Controlled Variables
- **Alphabet Size**: Tests impact of character set diversity
- **Dataset Size**: Evaluates scalability characteristics  
- **Query Complexity**: From simple 2-segment to complex 3-segment patterns
- **Frequency Distribution**: Simulates real-world data skew

### Realistic Patterns
- Queries are extracted from actual dataset strings
- Gap sizes and literal lengths based on practical regex usage
- Multiple sampling strategies to avoid overfitting

### Comprehensive Coverage
- Multiple experimental dimensions
- Various workload sizes for thorough testing
- Both training and test query sets

## Notes

- All experiments use reproducible random seeds for consistent results
- String generation includes safeguards for minimum lengths and valid ranges
- Query generation ensures meaningful patterns with adequate literal content
- File I/O includes proper error handling and directory creation

This synthetic workload suite provides comprehensive coverage for evaluating regex indexing algorithms across multiple performance dimensions and realistic usage scenarios.
