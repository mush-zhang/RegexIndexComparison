# Dataset Reading Methods Summary

## Updated Analysis Tools

I've updated both the C++ and Python analysis tools to correctly handle different dataset reading methods as implemented in `utils.cpp`:

### Reading Method Details

#### 1. Enron Dataset (workload case 6)
- **C++ Function:** `read_enron()`
- **Method:** File-per-document reading
- **Logic:** Each file becomes one string, preserving internal newlines
- **Usage:** 
  - C++: `./analyze_dataset_stats.out --enron -d data/enron/maildir`
  - Python: `python3 analyze_datasets.py --enron data/enron/maildir`

#### 2. Sysy Dataset (workload case 5) 
- **C++ Function:** `read_directory()` 
- **Method:** Line-by-line reading
- **Logic:** Each line in each file becomes a separate string
- **Usage:**
  - C++: `./analyze_dataset_stats.out --sysy -d data/extracted`
  - Python: `python3 analyze_datasets.py --sysy data/extracted`

#### 3. Other Datasets
- **C++ Function:** `read_directory()` (line-by-line) or `read_file_to_string()` (file-per-document)
- **Method:** Depends on dataset type
- **Usage:** Default is line-by-line unless `--enron` flag is used

## Key Changes Made

### C++ (`analyze_dataset_stats.cpp`)
- ✅ `--sysy` flag now uses line-by-line reading (`use_enron_reading = false`)
- ✅ Updated help text to clarify reading methods
- ✅ Auto-detection for Enron datasets remains file-per-document

### Python (`analyze_datasets.py`)
- ✅ `analyze_sysy_dataset()` now uses line-by-line reading
- ✅ Added separate `--max-lines` parameter for Sysy
- ✅ Updated help text and documentation
- ✅ Maintains `--max-files` for Enron and generic datasets

### Python (`analyze_results.py`)
- ✅ Added explicit handling for `sysy` dataset type
- ✅ Uses line-by-line reading for Sysy datasets

### Documentation (`DATASET_ANALYSIS.md`)
- ✅ Updated examples and usage instructions
- ✅ Clarified reading method differences
- ✅ Fixed dataset type associations

## Verification Commands

### Test Both Reading Methods:
```bash
# Test Enron-style reading (if data available)
./analyze_dataset_stats.out --enron -d data/enron/maildir -o enron_stats.csv

# Test Sysy-style reading (if data available)  
./analyze_dataset_stats.out --sysy -d data/extracted -o sysy_stats.csv

# Compare same dataset with different reading methods
./analyze_dataset_stats.out --enron -d data/some_test_dir -o test_enron_style.csv
./analyze_dataset_stats.out -d data/some_test_dir -o test_line_by_line.csv
```

### Python Analysis:
```bash
# Analyze both datasets with correct methods
python3 analyze_datasets.py --all --verbose

# Individual analysis with appropriate limits
python3 analyze_datasets.py --enron data/enron/maildir --max-files 1000
python3 analyze_datasets.py --sysy data/extracted --max-lines 10000
```

## Expected Behavior Differences

### Enron Reading vs Line-by-Line Reading

**Same dataset, different reading methods will show:**
- **Enron method:** Fewer "strings" (one per file), longer average length
- **Line-by-line method:** More "strings" (one per line), shorter average length

**Example:**
- File with 100 lines, 50 chars each = 5000 total chars
- **Enron reading:** 1 string, 5000 chars avg length
- **Line-by-line reading:** 100 strings, 50 chars avg length

## Consistency with C++ Benchmarking

The analysis tools now correctly mirror the reading logic used in the C++ benchmarking code:

| Workload | C++ Function | Reading Method | Analysis Flag |
|----------|--------------|----------------|---------------|
| Enron (6) | `read_enron()` | File-per-document | `--enron` |
| Sysy (5) | `read_directory()` | Line-by-line | `--sysy` |
| Others | `read_directory()` | Line-by-line | default |

This ensures that dataset statistics accurately reflect the data as it will be processed during actual benchmarking.
