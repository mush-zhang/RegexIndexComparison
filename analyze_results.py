#!/usr/bin/env python3
"""
Comprehensive Results Analysis Tool

This script analyzes benchmark results, dataset statistics, and workload summaries
for regex index comparison experiments. It supports Enron, synthetic, and other workloads
using the same file reading logic as the C++ benchmarking tools.
"""

import os
import sys
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import seaborn as sns
import argparse
import json
from pathlib import Path
from collections import defaultdict
import glob

def read_enron_file_as_string(filepath):
    """
    Read a file exactly like the C++ read_enron method:
    - Read entire file content as one string, preserving newlines
    - Remove carriage returns but keep the content
    """
    try:
        with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
            lines = []
            for line in f:
                # Remove carriage returns but keep the content
                line = line.replace('\r', '')
                lines.append(line.rstrip('\n'))
            
            # Join lines with newlines (like C++ version)
            content = '\n'.join(lines)
            return content
    except Exception as e:
        print(f"Error reading {filepath}: {e}")
        return ""

def read_enron_directory(data_path, max_files=-1):
    """
    Read Enron directory using the same logic as C++ read_enron method:
    Each file becomes one string in the dataset (preserving internal newlines)
    """
    data_strings = []
    
    if not os.path.exists(data_path):
        print(f"Enron directory not found: {data_path}")
        return data_strings
    
    file_count = 0
    for root, dirs, files in os.walk(data_path):
        for file in files:
            filepath = os.path.join(root, file)
            
            # Only process regular files
            if os.path.isfile(filepath):
                content = read_enron_file_as_string(filepath)
                if content:  # Only add non-empty content
                    data_strings.append(content)
                    
                file_count += 1
                if file_count % 100 == 0:
                    print(f"Processed {file_count} files, {len(data_strings)} non-empty documents...")
                
                # Check if we've reached the maximum number of files
                if max_files > 0 and len(data_strings) >= max_files:
                    break
        
        if max_files > 0 and len(data_strings) >= max_files:
            break
    
    print(f"Finished reading Enron. Total files: {file_count}, Total documents: {len(data_strings)}")
    return data_strings

def read_directory_line_by_line(data_path, max_lines=-1):
    """
    Read directory files line by line (for non-Enron datasets)
    """
    all_lines = []
    
    if not os.path.exists(data_path):
        print(f"Directory not found: {data_path}")
        return all_lines
    
    file_count = 0
    for root, dirs, files in os.walk(data_path):
        for file in files:
            filepath = os.path.join(root, file)
            
            if os.path.isfile(filepath):
                try:
                    with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
                        for line in f:
                            line = line.rstrip('\n\r')
                            all_lines.append(line)
                            
                            if max_lines > 0 and len(all_lines) >= max_lines:
                                break
                    
                    file_count += 1
                    if file_count % 100 == 0:
                        print(f"Processed {file_count} files, {len(all_lines)} lines...")
                    
                    if max_lines > 0 and len(all_lines) >= max_lines:
                        break
                        
                except Exception as e:
                    print(f"Error reading {filepath}: {e}")
        
        if max_lines > 0 and len(all_lines) >= max_lines:
            break
    
    print(f"Finished reading directory. Total files: {file_count}, Total lines: {len(all_lines)}")
    return all_lines

def analyze_dataset_statistics(data_strings, dataset_name, is_enron=False):
    """
    Analyze dataset statistics using the correct data format
    """
    stats = {
        'dataset_name': dataset_name,
        'is_enron': is_enron,
        'total_documents' if is_enron else 'total_lines': len(data_strings),
        'total_characters': sum(len(s) for s in data_strings),
        'alphabet_size': 0,
        'avg_length': 0.0,
        'min_length': float('inf') if data_strings else 0,
        'max_length': 0,
        'char_frequency': defaultdict(int),
        'alphabet': set()
    }
    
    if not data_strings:
        return stats
    
    # Analyze each string/document
    lengths = []
    for data_string in data_strings:
        length = len(data_string)
        lengths.append(length)
        
        stats['min_length'] = min(stats['min_length'], length)
        stats['max_length'] = max(stats['max_length'], length)
        
        # Count characters and build alphabet
        for char in data_string:
            stats['alphabet'].add(char)
            stats['char_frequency'][char] += 1
    
    stats['alphabet_size'] = len(stats['alphabet'])
    stats['avg_length'] = np.mean(lengths) if lengths else 0.0
    stats['length_std'] = np.std(lengths) if lengths else 0.0
    stats['length_median'] = np.median(lengths) if lengths else 0.0
    
    if stats['min_length'] == float('inf'):
        stats['min_length'] = 0
    
    return stats

def load_benchmark_results(results_dir):
    """
    Load benchmark results from CSV files
    """
    results_path = Path(results_dir)
    
    if not results_path.exists():
        print(f"Results directory not found: {results_dir}")
        return {}
    
    benchmark_data = {}
    
    # Look for CSV files with benchmark results
    csv_files = list(results_path.glob("*.csv"))
    
    for csv_file in csv_files:
        try:
            # Try to determine the file type based on name and content
            filename = csv_file.stem
            
            if 'time' in filename.lower() or 'benchmark' in filename.lower():
                # Load timing/benchmark results
                df = pd.read_csv(csv_file)
                benchmark_data[filename] = df
                print(f"Loaded benchmark results: {filename} ({len(df)} records)")
            
            elif 'dataset' in filename.lower() or 'stats' in filename.lower():
                # Load dataset statistics
                try:
                    df = pd.read_csv(csv_file, header=None, names=['metric', 'value'])
                    stats_dict = dict(zip(df['metric'], df['value']))
                    benchmark_data[filename] = stats_dict
                    print(f"Loaded dataset stats: {filename} ({len(stats_dict)} metrics)")
                except:
                    # Try with headers
                    df = pd.read_csv(csv_file)
                    benchmark_data[filename] = df
                    print(f"Loaded data file: {filename} ({len(df)} records)")
            
            else:
                # Generic CSV loading
                df = pd.read_csv(csv_file)
                benchmark_data[filename] = df
                print(f"Loaded generic CSV: {filename} ({len(df)} records)")
                
        except Exception as e:
            print(f"Error loading {csv_file}: {e}")
    
    return benchmark_data

def analyze_workload_summary(results_dir):
    """
    Analyze and summarize all workload results in a directory
    """
    results_path = Path(results_dir)
    
    print(f"\n=== WORKLOAD ANALYSIS SUMMARY ===")
    print(f"Results directory: {results_dir}")
    print(f"{'=' * 60}")
    
    # Load all benchmark data
    benchmark_data = load_benchmark_results(results_dir)
    
    if not benchmark_data:
        print("No benchmark data found.")
        return
    
    # Analyze dataset statistics
    dataset_stats = {}
    for filename, data in benchmark_data.items():
        if isinstance(data, dict) and any(key in filename.lower() for key in ['dataset', 'stats']):
            dataset_stats[filename] = data
    
    if dataset_stats:
        print(f"\n--- Dataset Statistics ---")
        for name, stats in dataset_stats.items():
            print(f"\n{name}:")
            if isinstance(stats, dict):
                for key, value in stats.items():
                    if isinstance(value, (int, float)):
                        if isinstance(value, float):
                            print(f"  {key}: {value:.2f}")
                        else:
                            print(f"  {key}: {value:,}")
                    else:
                        print(f"  {key}: {value}")
    
    # Analyze benchmark results
    benchmark_results = {}
    for filename, data in benchmark_data.items():
        if isinstance(data, pd.DataFrame) and any(key in filename.lower() for key in ['time', 'benchmark']):
            benchmark_results[filename] = data
    
    if benchmark_results:
        print(f"\n--- Benchmark Results ---")
        for name, df in benchmark_results.items():
            print(f"\n{name}:")
            print(f"  Records: {len(df)}")
            print(f"  Columns: {list(df.columns)}")
            
            # Try to extract timing information
            timing_cols = [col for col in df.columns if any(keyword in col.lower() 
                          for keyword in ['time', 'latency', 'duration', 'ms', 'seconds'])]
            
            if timing_cols:
                for col in timing_cols:
                    if df[col].dtype in ['float64', 'int64']:
                        print(f"  {col}: mean={df[col].mean():.3f}, std={df[col].std():.3f}")
    
    return benchmark_data

def create_analysis_plots(benchmark_data, output_dir=None):
    """
    Create analysis plots from benchmark data
    """
    if not benchmark_data:
        print("No data available for plotting.")
        return
    
    if output_dir:
        os.makedirs(output_dir, exist_ok=True)
    
    plt.style.use('default')
    
    # Plot benchmark timing results
    timing_data = {}
    for filename, data in benchmark_data.items():
        if isinstance(data, pd.DataFrame):
            timing_cols = [col for col in data.columns if any(keyword in col.lower() 
                          for keyword in ['time', 'latency', 'duration'])]
            if timing_cols:
                timing_data[filename] = data
    
    if timing_data:
        fig, axes = plt.subplots(len(timing_data), 1, figsize=(12, 6*len(timing_data)))
        if len(timing_data) == 1:
            axes = [axes]
        
        for i, (name, df) in enumerate(timing_data.items()):
            timing_cols = [col for col in df.columns if any(keyword in col.lower() 
                          for keyword in ['time', 'latency', 'duration'])]
            
            for col in timing_cols:
                if df[col].dtype in ['float64', 'int64']:
                    axes[i].hist(df[col], bins=30, alpha=0.7, label=col)
            
            axes[i].set_title(f'Timing Distribution - {name}')
            axes[i].set_xlabel('Time')
            axes[i].set_ylabel('Frequency')
            axes[i].legend()
        
        plt.tight_layout()
        
        if output_dir:
            plt.savefig(os.path.join(output_dir, 'timing_distributions.png'), dpi=300, bbox_inches='tight')
            print(f"Saved timing distribution plot to {output_dir}")
        else:
            plt.show()
        
        plt.close()

def main():
    parser = argparse.ArgumentParser(description='Analyze benchmark results and dataset statistics')
    parser.add_argument('results_dir', help='Directory containing benchmark results')
    parser.add_argument('--dataset-analysis', action='store_true', 
                        help='Perform dataset analysis (requires dataset path)')
    parser.add_argument('--dataset-path', help='Path to dataset directory for analysis')
    parser.add_argument('--dataset-type', choices=['enron', 'sysy', 'other'], default='other',
                        help='Type of dataset (affects reading method)')
    parser.add_argument('--max-files', type=int, default=-1,
                        help='Maximum number of files to process for dataset analysis')
    parser.add_argument('--output-plots', help='Directory to save analysis plots')
    parser.add_argument('--verbose', '-v', action='store_true', help='Verbose output')
    
    args = parser.parse_args()
    
    if args.verbose:
        print("Starting comprehensive analysis...")
    
    # Analyze benchmark results
    print("=== BENCHMARK RESULTS ANALYSIS ===")
    benchmark_data = analyze_workload_summary(args.results_dir)
    
    # Dataset analysis if requested
    if args.dataset_analysis and args.dataset_path:
        print(f"\n=== DATASET ANALYSIS ===")
        
        if args.dataset_type == 'enron':
            print("Using Enron-specific file reading (each file as one document)")
            data_strings = read_enron_directory(args.dataset_path, args.max_files)
            stats = analyze_dataset_statistics(data_strings, "Enron", is_enron=True)
        elif args.dataset_type == 'sysy':
            print("Using Sysy-specific file reading (line-by-line)")
            data_strings = read_directory_line_by_line(args.dataset_path, args.max_files)
            stats = analyze_dataset_statistics(data_strings, "Sysy", is_enron=False)
        else:
            print("Using line-by-line file reading")
            data_strings = read_directory_line_by_line(args.dataset_path, args.max_files)
            stats = analyze_dataset_statistics(data_strings, args.dataset_type, is_enron=False)
        
        print(f"\nDataset Statistics for {stats['dataset_name']}:")
        print(f"  Total {'documents' if stats['is_enron'] else 'lines'}: {stats['total_documents' if stats['is_enron'] else 'total_lines']:,}")
        print(f"  Total characters: {stats['total_characters']:,}")
        print(f"  Alphabet size: {stats['alphabet_size']} unique characters")
        print(f"  Average length: {stats['avg_length']:.2f} characters")
        print(f"  Length std dev: {stats['length_std']:.2f}")
        print(f"  Minimum length: {stats['min_length']} characters")
        print(f"  Maximum length: {stats['max_length']} characters")
        print(f"  Median length: {stats['length_median']:.2f} characters")
        
        # Show top characters
        top_chars = sorted(stats['char_frequency'].items(), key=lambda x: x[1], reverse=True)[:10]
        print(f"\nTop 10 most frequent characters:")
        for char, count in top_chars:
            char_repr = repr(char) if char in ['\n', '\t', '\r', ' '] else char
            print(f"  {char_repr}: {count:,} ({100*count/stats['total_characters']:.2f}%)")
    
    # Create plots if requested
    if args.output_plots:
        print(f"\n=== CREATING PLOTS ===")
        create_analysis_plots(benchmark_data, args.output_plots)
    
    print(f"\nAnalysis complete!")

if __name__ == "__main__":
    main()
