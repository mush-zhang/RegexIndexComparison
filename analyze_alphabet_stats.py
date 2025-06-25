#!/usr/bin/env python3
"""
Enron and Dataset Alphabet Analysis Tool

This script analyzes alphabet size, character frequency, and line length statistics
from dataset analysis results, with a focus on the Enron workload.
"""

import os
import sys
import pandas as pd
import argparse
from pathlib import Path
import json

def load_dataset_stats(csv_file):
    """Load dataset statistics from CSV file"""
    try:
        df = pd.read_csv(csv_file, header=None, names=['metric', 'value'])
        stats = {}
        for _, row in df.iterrows():
            stats[row['metric']] = row['value']
        return stats
    except Exception as e:
        print(f"Error loading {csv_file}: {e}")
        return {}

def load_literal_stats(csv_file):
    """Load regex literal statistics from CSV file"""
    try:
        # Read the CSV, skipping potential summary lines at the end
        df = pd.read_csv(csv_file)
        
        # If it has the expected columns for per-regex analysis
        if 'regex' in df.columns:
            return df
        else:
            # Try to read as simple format
            return pd.read_csv(csv_file, header=None, names=['metric', 'value'])
    except Exception as e:
        print(f"Error loading {csv_file}: {e}")
        return pd.DataFrame()

def analyze_alphabet_statistics(results_dir):
    """Analyze alphabet statistics from all result files"""
    results_path = Path(results_dir)
    
    if not results_path.exists():
        print(f"Results directory not found: {results_dir}")
        return
    
    print("=== ALPHABET SIZE AND LINE LENGTH ANALYSIS ===")
    print()
    
    # Find all dataset statistics files
    dataset_files = list(results_path.glob("*_dataset_stats.csv"))
    literal_files = list(results_path.glob("*_literal_analysis.csv"))
    
    alphabet_summary = []
    
    # Process dataset files
    for file in dataset_files:
        dataset_name = file.stem.replace("_dataset_stats", "")
        stats = load_dataset_stats(file)
        
        if stats:
            alphabet_summary.append({
                'dataset': dataset_name,
                'type': 'dataset',
                'alphabet_size': stats.get('alphabet_size', 'N/A'),
                'avg_line_length': stats.get('avg_line_length', 'N/A'),
                'total_lines': stats.get('total_lines', 'N/A'),
                'total_characters': stats.get('total_characters', 'N/A')
            })
    
    # Process literal analysis files
    for file in literal_files:
        workload_name = file.stem.replace("_literal_analysis", "")
        df = load_literal_stats(file)
        
        if not df.empty:
            # Try to extract summary statistics
            if 'regex' in df.columns and 'literal_count' in df.columns:
                avg_literals = df['literal_count'].mean() if 'literal_count' in df.columns else 'N/A'
                total_regexes = len(df)
            else:
                # Summary format
                avg_literals = 'N/A'
                total_regexes = 'N/A'
            
            alphabet_summary.append({
                'dataset': workload_name,
                'type': 'workload',
                'total_regexes': total_regexes,
                'avg_literals_per_regex': avg_literals,
                'file': file.name
            })
    
    # Display results
    print(f"{'Dataset/Workload':<20} {'Type':<10} {'Key Statistics':<50}")
    print("-" * 80)
    
    for item in alphabet_summary:
        if item['type'] == 'dataset':
            stats_str = f"Alphabet: {item['alphabet_size']}, Avg Length: {item['avg_line_length']:.2f}" if isinstance(item['avg_line_length'], (int, float)) else f"Alphabet: {item['alphabet_size']}, Avg Length: {item['avg_line_length']}"
        else:
            stats_str = f"Regexes: {item['total_regexes']}, Avg Literals: {item['avg_literals_per_regex']:.2f}" if isinstance(item['avg_literals_per_regex'], (int, float)) else f"Regexes: {item['total_regexes']}, Avg Literals: {item['avg_literals_per_regex']}"
        
        print(f"{item['dataset']:<20} {item['type']:<10} {stats_str}")
    
    print()
    
    # Focus on Enron results
    enron_datasets = [item for item in alphabet_summary if 'enron' in item['dataset'].lower()]
    
    if enron_datasets:
        print("=== ENRON SPECIFIC ANALYSIS ===")
        print()
        
        for item in enron_datasets:
            print(f"Enron {item['dataset']}:")
            if item['type'] == 'dataset':
                print(f"  - Alphabet Size: {item['alphabet_size']} unique characters")
                print(f"  - Average Line Length: {item['avg_line_length']} characters")
                print(f"  - Total Lines: {item['total_lines']}")
                print(f"  - Total Characters: {item['total_characters']}")
            else:
                print(f"  - Total Regexes: {item['total_regexes']}")
                print(f"  - Average Literals per Regex: {item['avg_literals_per_regex']}")
            print()
    else:
        print("No Enron-specific results found.")
        print("Make sure to run the analysis script first: ./analyze_enron_workload.sh")
    
    # Show available files for reference
    print("=== AVAILABLE RESULT FILES ===")
    all_files = list(results_path.glob("*.csv"))
    if all_files:
        for file in sorted(all_files):
            print(f"  - {file.name}")
    else:
        print("  No CSV result files found.")
    
    return alphabet_summary

def detailed_file_analysis(file_path):
    """Provide detailed analysis of a specific result file"""
    file_path = Path(file_path)
    
    if not file_path.exists():
        print(f"File not found: {file_path}")
        return
    
    print(f"=== DETAILED ANALYSIS: {file_path.name} ===")
    print()
    
    if 'dataset_stats' in file_path.name:
        stats = load_dataset_stats(file_path)
        
        print("Dataset Statistics:")
        for key, value in stats.items():
            if isinstance(value, float):
                print(f"  {key}: {value:.2f}")
            else:
                print(f"  {key}: {value}")
    
    elif 'literal_analysis' in file_path.name:
        df = load_literal_stats(file_path)
        
        if not df.empty:
            print("Regex Literal Analysis:")
            if 'regex' in df.columns:
                print(f"  Total regexes analyzed: {len(df)}")
                if 'literal_count' in df.columns:
                    print(f"  Average literals per regex: {df['literal_count'].mean():.2f}")
                    print(f"  Min literals: {df['literal_count'].min()}")
                    print(f"  Max literals: {df['literal_count'].max()}")
                
                # Show first few examples
                print("\n  Sample regexes:")
                for i, row in df.head(3).iterrows():
                    regex = row['regex'][:50] + "..." if len(row['regex']) > 50 else row['regex']
                    literals = row.get('literal_count', 'N/A')
                    print(f"    {regex} -> {literals} literals")
            else:
                print("  Summary statistics:")
                for _, row in df.iterrows():
                    print(f"    {row.iloc[0]}: {row.iloc[1]}")

def main():
    parser = argparse.ArgumentParser(description='Analyze alphabet size and statistics from regex workload results')
    parser.add_argument('results_dir', nargs='?', default='analysis_results',
                       help='Directory containing analysis result files (default: analysis_results)')
    parser.add_argument('--file', '-f', help='Analyze a specific result file in detail')
    parser.add_argument('--export', '-e', help='Export summary to JSON file')
    
    args = parser.parse_args()
    
    if args.file:
        detailed_file_analysis(args.file)
    else:
        summary = analyze_alphabet_statistics(args.results_dir)
        
        if args.export and summary:
            with open(args.export, 'w') as f:
                json.dump(summary, f, indent=2)
            print(f"\nSummary exported to: {args.export}")

if __name__ == '__main__':
    main()
