# Dataset Analysis Tools

This directory contains several tools for analyzing regex datasets and benchmark results, with specific support for Enron and Sysy workloads using the same file reading logic as the C++ benchmarking tools.

## Available Analysis Tools

### 1. `analyze_datasets.py` - Primary Dataset Analysis Tool

Analyzes datasets to count strings, calculate average string length, and determine alphabet size.

**Key Features:**
- Uses the same file reading logic as C++ `read_enron` method
- Each file is treated as one document/string (preserves internal newlines)
- Supports Enron, Sysy, and generic datasets
- Outputs comprehensive statistics including alphabet analysis

**Usage Examples:**
```bash
# Analyze Enron dataset
python3 analyze_datasets.py --enron data/enron/maildir

# Analyze Sysy dataset  
python3 analyze_datasets.py --sysy data/extracted

# Analyze both datasets and compare
python3 analyze_datasets.py --all --verbose

# Analyze with file limit and save results
python3 analyze_datasets.py --enron data/enron/maildir --max-files 1000 --output enron_stats.json

# Analyze a custom dataset
python3 analyze_datasets.py --dataset path/to/data --dataset-name "MyDataset"
```

### 2. `analyze_results.py` - Comprehensive Results Analysis

Analyzes benchmark results, dataset statistics, and workload summaries.

**Features:**
- Loads and analyzes CSV benchmark results
- Supports both Enron-style and line-by-line dataset analysis
- Creates timing distribution plots
- Comprehensive workload summaries

**Usage Examples:**
```bash
# Analyze benchmark results only
python3 analyze_results.py analysis_results/

# Analyze results with dataset analysis
python3 analyze_results.py analysis_results/ --dataset-analysis --dataset-path data/enron/maildir --dataset-type enron

# Generate plots
python3 analyze_results.py analysis_results/ --output-plots plots/
```

### 3. `analyze_dataset_stats.cpp` - C++ Dataset Analyzer

C++ tool that provides detailed dataset statistics with support for both reading modes.

**Compile:**
```bash
make analyze_dataset_stats.out
```

**Usage Examples:**
```bash
# Analyze Enron dataset (auto-detects Enron reading mode)
./analyze_dataset_stats.out -d data/enron/maildir

# Force Enron-style reading for any dataset
./analyze_dataset_stats.out -d data/extracted --enron -n "Sysy"

# Analyze with output file
./analyze_dataset_stats.out -d data/enron/maildir -o enron_stats.csv

# Line-by-line analysis for comparison
./analyze_dataset_stats.out -d data/some_dataset -n "MyDataset"
```

### 4. `analyze_enron_workload.sh` - Automated Analysis Pipeline

Shell script that runs comprehensive analysis of Enron and other workloads.

**Usage:**
```bash
./analyze_enron_workload.sh
```

**What it does:**
- Builds required analysis tools
- Analyzes regex workload files (literal analysis)
- Analyzes dataset directories (alphabet and statistics)
- Runs comprehensive Python analysis
- Generates summary reports

## File Reading Methods

### Enron-Style Reading
- **Used for:** Enron emails, Sysy dataset, and other document collections
- **Method:** Each file becomes one string in the dataset
- **Preserves:** Internal newlines within documents
- **Example:** An email file with multiple lines becomes one long string with embedded `\n` characters

### Line-by-Line Reading  
- **Used for:** Traditional text datasets
- **Method:** Each line in each file becomes a separate string
- **Example:** A file with 10 lines becomes 10 separate strings in the dataset

## Key Metrics Analyzed

### Primary Metrics
- **Number of strings/documents:** Total count of data items
- **Average string length:** Mean character count per string/document
- **Alphabet size:** Number of unique characters across entire dataset

### Additional Metrics (detailed analysis)
- Total character count
- Min/max string lengths
- Length distribution (percentiles)
- Character frequency analysis
- Special character identification

## Output Formats

### JSON Output (analyze_datasets.py)
```json
{
  "dataset_name": "Enron",
  "num_strings": 12345,
  "avg_string_length": 1234.56,
  "alphabet_size": 95,
  "total_characters": 15234567,
  "alphabet": ["!", "\"", "#", ...],
  "reading_method": "file-per-document"
}
```

### CSV Output (C++ tool)
```csv
dataset_name,total_lines,total_characters,alphabet_size,avg_line_length,min_line_length,max_line_length,median_line_length
Enron,12345,15234567,95,1234.56,1,45678,892
```

## Common Use Cases

### 1. Dataset Characterization
```bash
# Get basic statistics for Enron dataset
python3 analyze_datasets.py --enron data/enron/maildir --verbose

# Compare multiple datasets
python3 analyze_datasets.py --all --output comparison.json
```

### 2. Benchmark Analysis
```bash
# Analyze benchmark results with dataset context
python3 analyze_results.py results/ --dataset-analysis --dataset-path data/enron/maildir --dataset-type enron
```

### 3. Quick Dataset Check
```bash
# Quick analysis with file limit
python3 analyze_datasets.py --enron data/enron/maildir --max-files 100
```

### 4. Automated Pipeline
```bash
# Run full analysis pipeline
./analyze_enron_workload.sh
```

## Notes

- **File Reading Consistency:** All tools use the same file reading logic as the C++ benchmarking code
- **Unicode Support:** All Python tools handle Unicode characters with error tolerance
- **Memory Efficiency:** Tools process files incrementally when possible
- **Error Handling:** Robust error handling for missing files, permissions, etc.

For more details, see the individual script help messages:
```bash
python3 analyze_datasets.py --help
python3 analyze_results.py --help
./analyze_dataset_stats.out -h
```
