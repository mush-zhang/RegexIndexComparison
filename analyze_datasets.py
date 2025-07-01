#!/usr/bin/env python3
"""
Enron and Sysy Dataset Analysis Script

This script analyzes Enron and Sysy datasets to count:
- Number of strings/documents in the dataset
- Average string length
- Alphabet size (unique characters)

Uses the same file reading logic as the C++ benchmarking tools.
"""

import os
import sys
import argparse
from pathlib import Path
from collections import Counter
import json

def read_file_as_string(filepath):
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

def analyze_enron_dataset(data_path, max_files=-1, verbose=False):
    """
    Analyze Enron dataset using file-per-document reading
    """
    if not os.path.exists(data_path):
        print(f"Enron directory not found: {data_path}")
        return None
    
    print(f"Analyzing Enron dataset: {data_path}")
    print("Using Enron-style reading (each file as one document)...")
    
    data_strings = []
    alphabet = set()
    total_chars = 0
    
    file_count = 0
    for root, dirs, files in os.walk(data_path):
        for file in files:
            filepath = os.path.join(root, file)
            
            # Only process regular files
            if os.path.isfile(filepath):
                content = read_file_as_string(filepath)
                if content:  # Only add non-empty content
                    data_strings.append(content)
                    total_chars += len(content)
                    
                    # Add characters to alphabet
                    alphabet.update(content)
                    
                file_count += 1
                if verbose and file_count % 1000 == 0:
                    print(f"  Processed {file_count} files, {len(data_strings)} non-empty documents...")
                
                # Check if we've reached the maximum number of files
                if max_files > 0 and len(data_strings) >= max_files:
                    break
        
        if max_files > 0 and len(data_strings) >= max_files:
            break
    
    if verbose:
        print(f"Finished reading Enron. Total files: {file_count}, Total documents: {len(data_strings)}")
    
    # Calculate statistics
    num_strings = len(data_strings)
    avg_length = total_chars / num_strings if num_strings > 0 else 0
    alphabet_size = len(alphabet)
    
    return {
        'dataset_name': 'Enron',
        'dataset_path': data_path,
        'num_strings': num_strings,
        'total_characters': total_chars,
        'avg_string_length': avg_length,
        'alphabet_size': alphabet_size,
        'alphabet': sorted(list(alphabet)),
        'reading_method': 'file-per-document'
    }

def analyze_sysy_dataset(data_path, max_files=-1, verbose=False):
    """
    Analyze Sysy dataset using file-per-document reading
    """
    if not os.path.exists(data_path):
        print(f"Sysy directory not found: {data_path}")
        return None
    
    print(f"Analyzing Sysy dataset: {data_path}")
    print("Using file-per-document reading (same as Enron)...")
    
    data_strings = []
    alphabet = set()
    total_chars = 0
    
    file_count = 0
    for root, dirs, files in os.walk(data_path):
        for file in files:
            filepath = os.path.join(root, file)
            
            # Only process regular files
            if os.path.isfile(filepath):
                content = read_file_as_string(filepath)
                if content:  # Only add non-empty content
                    data_strings.append(content)
                    total_chars += len(content)
                    
                    # Add characters to alphabet
                    alphabet.update(content)
                    
                file_count += 1
                if verbose and file_count % 1000 == 0:
                    print(f"  Processed {file_count} files, {len(data_strings)} non-empty documents...")
                
                # Check if we've reached the maximum number of files
                if max_files > 0 and len(data_strings) >= max_files:
                    break
        
        if max_files > 0 and len(data_strings) >= max_files:
            break
    
    if verbose:
        print(f"Finished reading Sysy. Total files: {file_count}, Total documents: {len(data_strings)}")
    
    # Calculate statistics
    num_strings = len(data_strings)
    avg_length = total_chars / num_strings if num_strings > 0 else 0
    alphabet_size = len(alphabet)
    
    return {
        'dataset_name': 'Sysy',
        'dataset_path': data_path,
        'num_strings': num_strings,
        'total_characters': total_chars,
        'avg_string_length': avg_length,
        'alphabet_size': alphabet_size,
        'alphabet': sorted(list(alphabet)),
        'reading_method': 'file-per-document'
    }

def analyze_generic_dataset(data_path, dataset_name, max_files=-1, verbose=False):
    """
    Analyze a generic dataset using file-per-document reading
    """
    if not os.path.exists(data_path):
        print(f"Dataset directory not found: {data_path}")
        return None
    
    print(f"Analyzing {dataset_name} dataset: {data_path}")
    print("Using file-per-document reading...")
    
    data_strings = []
    alphabet = set()
    total_chars = 0
    
    file_count = 0
    for root, dirs, files in os.walk(data_path):
        for file in files:
            filepath = os.path.join(root, file)
            
            # Only process regular files
            if os.path.isfile(filepath):
                content = read_file_as_string(filepath)
                if content:  # Only add non-empty content
                    data_strings.append(content)
                    total_chars += len(content)
                    
                    # Add characters to alphabet
                    alphabet.update(content)
                    
                file_count += 1
                if verbose and file_count % 1000 == 0:
                    print(f"  Processed {file_count} files, {len(data_strings)} non-empty documents...")
                
                # Check if we've reached the maximum number of files
                if max_files > 0 and len(data_strings) >= max_files:
                    break
        
        if max_files > 0 and len(data_strings) >= max_files:
            break
    
    if verbose:
        print(f"Finished reading {dataset_name}. Total files: {file_count}, Total documents: {len(data_strings)}")
    
    # Calculate statistics
    num_strings = len(data_strings)
    avg_length = total_chars / num_strings if num_strings > 0 else 0
    alphabet_size = len(alphabet)
    
    return {
        'dataset_name': dataset_name,
        'dataset_path': data_path,
        'num_strings': num_strings,
        'total_characters': total_chars,
        'avg_string_length': avg_length,
        'alphabet_size': alphabet_size,
        'alphabet': sorted(list(alphabet)),
        'reading_method': 'file-per-document'
    }

def print_dataset_stats(stats):
    """
    Print dataset statistics in a formatted way
    """
    if not stats:
        print("No statistics available.")
        return
    
    print(f"\n{'=' * 60}")
    print(f"DATASET ANALYSIS: {stats['dataset_name']}")
    print(f"{'=' * 60}")
    print(f"Dataset Path: {stats['dataset_path']}")
    print(f"Reading Method: {stats['reading_method']}")
    print()
    print(f"Number of strings/documents: {stats['num_strings']:,}")
    print(f"Total characters: {stats['total_characters']:,}")
    print(f"Average string length: {stats['avg_string_length']:.2f} characters")
    print(f"Alphabet size: {stats['alphabet_size']} unique characters")
    print()
    
    # Show some alphabet characters
    alphabet = stats['alphabet']
    print("Alphabet sample (first 50 printable characters):")
    printable_chars = [c for c in alphabet if 32 <= ord(c) <= 126][:50]
    print("  " + " ".join(f"'{c}'" for c in printable_chars))
    if len(alphabet) > 50:
        print(f"  ... and {len(alphabet) - 50} more characters")
    
    # Show special characters
    special_chars = [c for c in alphabet if ord(c) < 32 or ord(c) > 126]
    if special_chars:
        print(f"\nSpecial/control characters found: {len(special_chars)}")
        common_special = []
        for c in special_chars[:10]:  # Show first 10
            if c == '\n':
                common_special.append('NEWLINE')
            elif c == '\t':
                common_special.append('TAB')
            elif c == ' ':
                common_special.append('SPACE')
            else:
                common_special.append(f'CTRL-{ord(c)}')
        print("  " + ", ".join(common_special))
        if len(special_chars) > 10:
            print(f"  ... and {len(special_chars) - 10} more")

def save_stats_to_file(stats, output_file):
    """
    Save statistics to a JSON file
    """
    try:
        # Convert alphabet set to list for JSON serialization
        stats_copy = stats.copy()
        with open(output_file, 'w', encoding='utf-8') as f:
            json.dump(stats_copy, f, indent=2, ensure_ascii=False)
        print(f"\nStatistics saved to: {output_file}")
    except Exception as e:
        print(f"Error saving statistics to {output_file}: {e}")

def main():
    parser = argparse.ArgumentParser(description='Analyze Enron and Sysy datasets for string count, average length, and alphabet size')
    parser.add_argument('--enron', help='Path to Enron dataset directory (e.g., data/enron/maildir)')
    parser.add_argument('--sysy', help='Path to Sysy dataset directory (e.g., data/extracted)')
    parser.add_argument('--dataset', help='Path to generic dataset directory')
    parser.add_argument('--dataset-name', help='Name for generic dataset (required with --dataset)')
    parser.add_argument('--max-files', type=int, default=-1, help='Maximum number of files to process')
    parser.add_argument('--output', help='Output file to save statistics (JSON format)')
    parser.add_argument('--verbose', '-v', action='store_true', help='Verbose output')
    parser.add_argument('--all', action='store_true', help='Analyze all available datasets')
    
    args = parser.parse_args()
    
    if not any([args.enron, args.sysy, args.dataset, args.all]):
        parser.error("Must specify at least one dataset to analyze (--enron, --sysy, --dataset, or --all)")
    
    if args.dataset and not args.dataset_name:
        parser.error("--dataset-name is required when using --dataset")
    
    all_stats = []
    
    # Analyze Enron dataset
    if args.enron or args.all:
        enron_path = args.enron if args.enron else "data/enron/maildir"
        enron_stats = analyze_enron_dataset(enron_path, args.max_files, args.verbose)
        if enron_stats:
            print_dataset_stats(enron_stats)
            all_stats.append(enron_stats)
        else:
            print("Failed to analyze Enron dataset.")
    
    # Analyze Sysy dataset
    if args.sysy or args.all:
        sysy_path = args.sysy if args.sysy else "data/extracted"
        sysy_stats = analyze_sysy_dataset(sysy_path, args.max_files, args.verbose)
        if sysy_stats:
            print_dataset_stats(sysy_stats)
            all_stats.append(sysy_stats)
        else:
            print("Failed to analyze Sysy dataset.")
    
    # Analyze generic dataset
    if args.dataset:
        generic_stats = analyze_generic_dataset(args.dataset, args.dataset_name, args.max_files, args.verbose)
        if generic_stats:
            print_dataset_stats(generic_stats)
            all_stats.append(generic_stats)
        else:
            print(f"Failed to analyze {args.dataset_name} dataset.")
    
    # Summary comparison if multiple datasets
    if len(all_stats) > 1:
        print(f"\n{'=' * 80}")
        print("DATASET COMPARISON SUMMARY")
        print(f"{'=' * 80}")
        print(f"{'Dataset':<15} {'Strings':<12} {'Avg Length':<12} {'Alphabet Size':<15} {'Total Chars':<15}")
        print("-" * 80)
        
        for stats in all_stats:
            print(f"{stats['dataset_name']:<15} {stats['num_strings']:<12,} "
                  f"{stats['avg_string_length']:<12.1f} {stats['alphabet_size']:<15} "
                  f"{stats['total_characters']:<15,}")
    
    # Save to file if requested
    if args.output:
        if len(all_stats) == 1:
            save_stats_to_file(all_stats[0], args.output)
        else:
            save_stats_to_file({'datasets': all_stats}, args.output)

if __name__ == "__main__":
    main()
