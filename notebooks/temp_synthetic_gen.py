#!/usr/bin/env python
# coding: utf-8

# In[1]:


import os
import sys
import numpy as np
import string
import random
import re
import pickle
from concurrent.futures import ThreadPoolExecutor, ProcessPoolExecutor, as_completed
from threading import Lock
import multiprocessing as mp
from functools import partial
import time
import pickle
import random
from collections import defaultdict, Counter
from multiprocessing import Process, Manager, Lock
import matplotlib.pyplot as plt


# In[2]:


DATA_DIR = '.' # '../data/synthetic/'


# In[3]:


random.seed(53711)
np.random.seed(15213)


# ## Expr 1

# In[4]:


directory_path = 'synthetic1'

# Parameters
dataset_size = 400_000  # Expected size of the dataset
means = [(800, 600), (1200, 400) , (100, 1900), (0, 3700)]  # Mean frequency for all datasets
std_devs = [100, 200, 300, 400]  # Four different standard deviations
query_counts = [random.randint(227, 248) for _ in range(4)]


# In[5]:


def generate_query_key():
    """Generate a random query key with length between 3 and 8 inclusively."""
    key_length = random.randint(3, 8)
    return ''.join(random.choices(string.ascii_uppercase, k=key_length))

def generate_query():
    """Generate a query with 3 query keys and 2 regex gap constraints."""
    # Generate 3 query keys
    key1 = generate_query_key()
    key2 = generate_query_key()
    key3 = generate_query_key()

    # Generate 2 random gap constraints
    gap1_lower = int(bool(random.getrandbits(1)))
    gap1_upper = random.randint(1, 50)
    gap2_lower = int(bool(random.getrandbits(1)))
    gap2_upper = random.randint(1, 50)

    # Format the query with regex gaps
    query = f"{key1}(.{{{gap1_lower},{gap1_upper}}}){key2}(.{{{gap2_lower},{gap2_upper}}}){key3}"
    return query

def generate_query_workload(query_count):
    """Generate a workload of queries."""
    return [generate_query() for _ in range(query_count)]

def generate_trigrams():
    """Generate all unique trigrams from A-Z."""
    alphabet = string.ascii_uppercase  # A-Z
    trigrams = [f"{a}{b}{c}" for a in alphabet for b in alphabet for c in alphabet]
    return trigrams  # Use the unique trigrams

def generate_dataset(trigrams, trigram_frequencies, dataset_size, lock, shared_dataset, index):
    """Generate a dataset dynamically based on trigram frequencies."""
    dataset = []
    trigram_counter = Counter()  # Track the appearance of each trigram

    # Generate strings until the dataset reaches the desired size
    while len(dataset) < dataset_size:
        current_string = []

        # Randomly select trigrams until the dataset is complete
        while True:
            # Select available trigrams based on remaining frequency
            available_trigrams = [
                t for t in trigrams if trigram_counter[t] < trigram_frequencies[t]
            ]
            if not available_trigrams:
                break  # No more trigrams can be selected

            # Randomly select a trigram to add to the current string
            next_trigram = random.choice(available_trigrams)
            current_string.append(next_trigram)
            trigram_counter[next_trigram] += 1

            # End string generation with some probability to avoid infinite strings
            if random.random() < 0.3:
                break

        # Ensure the string contains at least one trigram
        if current_string:
            dataset.append(''.join(current_string))

    # Save dataset to the shared manager list
    with lock:
        shared_dataset[index] = (dataset, trigram_counter)

def generate_frequencies(trigrams, mean1, mean2, std_dev):
    """Generate target frequencies for the trigrams using a Normal distribution."""
    size1 = int(len(trigrams)*random.uniform(0.9, 0.6)) # larger
    size2 = len(trigrams) - size1 # smaller
    frequencies1 = np.random.normal(loc=mean1, scale=std_dev, size=size1)
    frequencies2 = np.random.normal(loc=mean2, scale=std_dev, size=size2)

    # Ensure positive integer frequencies and avoid frequencies of 0
    frequencies1 = np.clip(np.abs(frequencies1).astype(int), 1, None)
    frequencies2 = np.clip(np.abs(frequencies2).astype(int), 1, None)

    frequencies = np.append(frequencies1, frequencies2)
    np.random.shuffle(frequencies)
    return dict(zip(trigrams, frequencies))

def generate_expr1():
    os.makedirs(directory_path, exist_ok=True)

    # Generate all trigrams
    trigrams = generate_trigrams()

    # Shared manager object to store datasets
    manager = Manager()
    shared_dataset = manager.list([None] * 4)  # Store 4 datasets
    lock = Lock()

    # Create and start processes to generate multiple datasets in parallel
    processes = []
    for i in range(4):
        frequencies = generate_frequencies(trigrams, means[i][0], means[i][1], std_devs[i])
        p = Process(target=generate_dataset, args=(
            trigrams, frequencies, dataset_size, lock, shared_dataset, i))
        processes.append(p)
        p.start()

    shared_queries = []
    # Generate query workloads
    for query_count in query_counts:
        queries = generate_query_workload(query_count)
        shared_queries.append(queries)
        print(len(shared_queries[-1]))

    # Wait for all processes to complete
    for p in processes:
        p.join()

    # Print summary of datasets and query workloads
    for i, (dataset, trigram_counter) in enumerate(shared_dataset):
        print(f"Dataset {i + 1} with std_dev {std_devs[i]}: {len(dataset)} strings")
        print(f"Top 10 Trigrams (by frequency): {trigram_counter.most_common(10)}")
        print(f"Bottom 10 Trigrams (by frequency): {trigram_counter.most_common()[:-10-1:-1]}")
        print(f"Query Workload {i + 1}: {len(shared_queries[i])} queries")
        print(f"Sample Query: {shared_queries[i][0]}")
        with open(os.path.join(directory_path, f'data_{i}_std{std_devs[i]}.pkl'), 'wb') as f:
            pickle.dump(dataset, f)
        with open(os.path.join(directory_path, f'query_{i}.pkl'), 'wb') as f:
            pickle.dump(shared_queries[i], f)

    datasets = [ dataset for dataset, trigram_counter in shared_dataset ]

    os.makedirs(os.path.join(DATA_DIR, 'expr1'), exist_ok=True)
    for i in range(4):
        query_fn = os.path.join(DATA_DIR, 'expr1', f'query_{i}.txt')
        if not os.path.exists(query_fn):
            with open(query_fn, 'w') as f:
                for line in shared_queries[i]:
                    f.write(line + '\n')
        data_fn = os.path.join(DATA_DIR, 'expr1', f'data_{i}_std{std_devs[i]}.txt')
        if not os.path.exists(data_fn):
            with open(data_fn, 'w') as f:
                for line in datasets[i]:
                    f.write(line + '\n')
    return datasets, shared_queries


# In[6]:


def analyze_dataset(dataset, trigrams):
    """
    Analyze the dataset to count how many strings each trigram appears in.
    """
    trigram_in_line = Counter()  # Track which trigrams appear in each string

    # Iterate over each string in the dataset and count trigram occurrences per string
    for line in dataset:
        unique_trigrams = set(
            line[i:i + 3] for i in range(len(line) - 2) if line[i:i + 3] in trigrams
        )
        for trigram in unique_trigrams:
            trigram_in_line[trigram] += 1

    return trigram_in_line

def plot_histogram(trigram_counter, dataset_index, std_dev_val, output_dir):
    """
    Plot a histogram for the distribution of trigrams across strings.
    """
    # Extract the frequency of each trigram (i.e., how many strings contain it)
    frequencies = list(trigram_counter.values())

    # Define bins for the histogram
    bins = np.arange(0, max(frequencies) + 5, 5)  # Binning every 5 occurrences

    # Plot the histogram
    plt.figure(figsize=(10, 6))
    plt.hist(frequencies, bins=bins, edgecolor='black', alpha=0.7)
    plt.xlabel('Number of Lines Appears')
    plt.ylabel('Number of Trigrams')
    plt.title(f'Trigram Distribution std_dev={std_dev_val}')

    # Save the plot as a PDF
    output_file = os.path.join(output_dir, f'dataset_{dataset_index + 1}_histogram.pdf')
    plt.savefig(output_file)
    plt.close()

def analysis_expr1(datasets, trigrams):
    """
    Analyze all datasets and generate histogram plots.
    """
    output_dir = 'histogram_plots'
    os.makedirs(output_dir, exist_ok=True)  # Create output directory if it doesn't exist

    # Analyze each dataset and generate corresponding histograms
    for i, dataset in enumerate(datasets):
        trigram_counter = analyze_dataset(dataset, trigrams)
        plot_histogram(trigram_counter, i,std_devs[i], output_dir)

    print(f'All histograms saved to: {output_dir}')


# #### datasets, shared_queries = generate_expr1()

# In[7]:


# if not os.path.isdir(directory_path):
#     datasets, shared_queries = generate_expr1()
# else:
#     datasets = []
#     shared_queries = []
#     for i in range(4):
#         with open(os.path.join(directory_path, f'data_{i}_std{std_devs[i]}.pkl'), 'rb') as f:
#             datasets.append(pickle.load(f))
#         with open(os.path.join(directory_path, f'query_{i}.pkl'), 'rb') as f:
#             shared_queries.append(pickle.load(f))
# analysis_expr1(datasets, trigrams)


# ## Expr 2

# In[20]:


def generate_dataset_expr2(num_strings, string_length=450):
    """Generate a dataset with specified number of strings, each of fixed length."""
    alphabet = string.ascii_uppercase  # English uppercase alphabet
    dataset = [
        ''.join(random.choices(alphabet, k=string_length))
        for _ in range(num_strings)
    ]
    return dataset

def sample_dataset(dataset, sample_percentage):
    """Take a sample of the dataset with the specified percentage."""
    sample_size = int(len(dataset) * sample_percentage)
    return random.sample(dataset, sample_size)

def generate_query_workload(sample, query_count, lower_char_count, upper_char_count):
    """Generate query workload from the given sample."""
    queries = []

    for _ in range(query_count):
        # Select a random string from the sample
        random_string = random.choice(sample)

        # Ensure the string has at least lower characters for meaningful slicing
        while len(random_string) < lower_char_count+1:
            random_string = random.choice(sample)

        # Choose a random slice of characters from the string
        slice1_start = random.randint(0, len(random_string) - lower_char_count)
        slice1_length = random.randint(lower_char_count, upper_char_count)
        slice1 = random_string[slice1_start:slice1_start + min(slice1_length, len(random_string)-1)]

        # Decide on a random gap size
        gap_size = random.randint(1, max(1, min(50, len(random_string) - len(slice1) - 1)))

        # Choose another slice of 3-8 characters after the gap
        slice2_start = slice1_start + slice1_length + gap_size
        if slice2_start + upper_char_count < len(random_string):  # Ensure valid slice range
            slice2_length = random.randint(lower_char_count, upper_char_count)
            slice2 = random_string[slice2_start:slice2_start + slice2_length]
        else:
            slice2 = random_string[slice2_start:]  # Handle edge case where slice2 is out of range

        # Create the query string with a regex-style gap
        query = f"{slice1}(.{{0,{gap_size}}}){slice2}"
        queries.append(query)

    return queries

def save_dataset(dataset, filename):
    """Save the dataset to a file."""
    with open(filename, 'w') as f:
        for line in dataset:
            f.write(line + '\n')

def save_queries(queries, filename):
    """Save the query workload to a file."""
    with open(filename, 'w') as f:
        for query in queries:
            f.write(query + '\n')

def generate_and_save_datasets(data_dir):
    """Generate datasets and save them to files."""
    dataset_sizes = [20_000, 40_000, 60_000, 80_000, 100_000]

    for size in dataset_sizes:
        dataset = generate_dataset_expr2(size)
        save_dataset(dataset, os.path.join(data_dir, f'dataset_{size}.txt'))

def generate_and_save_query_workloads(query_dir, data_dir):
    """Generate query workloads from datasets and save them."""
    query_sizes = [100, 500, 2_000, 2_500, 5_000]

    # Load the 20K dataset to generate workloads
    with open(os.path.join(data_dir, 'dataset_20000.txt'), 'r') as f:
        dataset_20k = [line.strip() for line in f.readlines()]

    for query_size in query_sizes:
        sample = sample_dataset(dataset_20k, 0.1)  # Take 10% sample
        queries = generate_query_workload(sample, query_size, 3, 8)
        save_queries(queries, os.path.join(query_dir, f'query_workload_{query_size}.txt'))

def generate_and_save_fixed_workload(query_dir, data_dir):
    """Generate a fixed 1000-query workload for all datasets."""
    # Generate workloads for the original datasets
    dataset_sizes = [20_000, 40_000, 60_000, 80_000, 100_000]

    for size in dataset_sizes:
        with open(os.path.join(data_dir, f'dataset_{size}.txt'), 'r') as f:
            dataset = [line.strip() for line in f.readlines()]

        sample = sample_dataset(dataset, 0.1)  # Take 10% sample
        queries = generate_query_workload(sample, 1000, 3, 8)  # Fixed 1000 queries
        save_queries(queries, os.path.join(query_dir, f'query_workload_1000_for_{size}.txt'))

def generate_expr2():
    query_dir = os.path.join(DATA_DIR, 'expr2', 'queries')
    os.makedirs(query_dir, exist_ok=True)
    data_dir = os.path.join(DATA_DIR, 'expr2', 'datasets')
    os.makedirs(data_dir, exist_ok=True)
    # Generate datasets
    print("Generating datasets...")
    generate_and_save_datasets(data_dir)

    # Generate query workloads with fixed 20K dataset
    print("Generating query workloads with fixed 20K dataset...")
    generate_and_save_query_workloads(query_dir, data_dir)

    # Generate fixed 1000-query workloads for all datasets
    print("Generating fixed 1000-query workloads...")
    generate_and_save_fixed_workload(query_dir, data_dir)

    print("All datasets and query workloads generated successfully!")


# In[21]:


# generate_expr2()


# ## Expr 4

# In[22]:


def generate_dataset_expr4(alphabet, num_records=5000):
    """Generate a dataset of strings using the given alphabet."""
    dataset = []

    for _ in range(num_records):
        current_string = []
        s = len(alphabet)  # Alphabet size

        # Generate the string character-by-character with 1/(2*s) chance of stopping
        while True:
            char = random.choice(alphabet)
            current_string.append(char)

            # End generation with a 1/(2*s) probability after adding a character
            if random.random() < 1 / (10 * s):
                break

        dataset.append(''.join(current_string))

    return dataset

def generate_expr4():
    # Define alphabets for Rob datasets
    rob_alphabets = {
        'Rob01': string.ascii_uppercase[:4],  # A-D
        'Rob02': string.ascii_uppercase[:8],  # A-H
        'Rob03': string.ascii_uppercase[:12],  # A-L
        'Rob04': string.ascii_uppercase[:16],  # A-P
    }

    data_dir = os.path.join('expr4', 'datasets')
    query_dir = os.path.join('expr4', 'queries')
    os.makedirs(data_dir, exist_ok=True)
    os.makedirs(query_dir, exist_ok=True)

    # Generate datasets for Rob01 to Rob04
    for rob_name, alphabet in rob_alphabets.items():
        dataset = generate_dataset_expr4(alphabet)
        save_dataset(dataset, os.path.join(data_dir, f'{rob_name}.txt'))

        # Generate 10%, 30%, and 50% samples and their query workloads
        for sample_pct in [0.1, 0.3, 0.5]:
            sample = sample_dataset(dataset, sample_pct)
            queries = generate_query_workload(sample, int(len(dataset) * sample_pct), 3, 8)
            save_queries(queries, os.path.join(query_dir, f'{rob_name}_queries_{int(sample_pct*100)}pct.txt'))
        # Generate 2% test query set
        test_sample = sample_dataset(dataset, 0.02)
        test_queries = generate_query_workload(sample, int(len(dataset) * 0.02), 3, 8)
        save_queries(test_queries, os.path.join(query_dir, f'{rob_name}_test_queries_2pct.txt'))


# In[23]:


generate_expr4()


# # Expr 5

# In[ ]:


def generate_dataset_expr5(alphabet, num_records=5000):
    """Generate a dataset of strings using the given alphabet."""
    dataset = []

    for _ in range(num_records):
        current_string = []
        s = len(alphabet)  # Alphabet size

        # Generate the string character-by-character with 1/(2*s) chance of stopping
        while True:
            char = random.choice(alphabet)
            current_string.append(char)

            if random.random() < 1 / (10 * s):
                break

        dataset.append(''.join(current_string))

    return dataset

def generate_query_workload5(sample, query_count, lower_char_count, upper_char_count):
    """Generate query workload from the given sample."""
    queries = []

    for _ in range(query_count):
        # Select a random string from the sample
        random_string = random.choice(sample)

        # Ensure the string has at least lower characters for meaningful slicing
        while len(random_string) < 3*lower_char_count+1:
            random_string = random.choice(sample)

        # Choose a random slice of characters from the string
        slice1_start = random.randint(0, len(random_string) - 3*lower_char_count-1)
        slice1_length = random.randint(lower_char_count, upper_char_count)
        slice1 = random_string[slice1_start:slice1_start + min(slice1_length, len(random_string)-1)]

        # Decide on a random gap size
        gap_size1 = random.randint(1, max(1, min(50, len(random_string) - len(slice1) - 1)))

        # Choose another slice of 3-8 characters after the gap
        slice2_start = slice1_start + slice1_length + gap_size1
        slice2_length = 0
        if slice2_start + upper_char_count < len(random_string):  # Ensure valid slice range
            slice2_length = random.randint(lower_char_count, upper_char_count)
            slice2 = random_string[slice2_start:slice2_start + slice2_length]
        else:
            slice2 = random_string[slice2_start:]  # Handle edge case where slice2 is out of range
            # Create the query string with a regex-style gap
            query = f"{slice1}(.{{0,{gap_size1}}}){slice2}"
            queries.append(query)
            continue

        # Decide on a random gap size
        gap_size2 = random.randint(1, max(1, min(50, len(random_string) - len(slice2) - gap_size1 - len(slice1) - 1)))

        # Choose another slice of 3-8 characters after the gap
        slice3_start = slice2_start + slice2_length + gap_size2
        if slice3_start + upper_char_count < len(random_string):  # Ensure valid slice range
            slice3_length = random.randint(lower_char_count, upper_char_count)
            slice3 = random_string[slice3_start:slice3_start + slice3_length]
        else:
            slice3 = random_string[slice3_start:]  # Handle edge case where slice3 is out of range

        # Create the query string with a regex-style gap
        query = f"{slice1}(.{{0,{gap_size1}}}){slice2}(.{{0,{gap_size2}}}){slice3}"
        queries.append(query)

    return queries


def generate_expr5():
    # Define alphabets for Rob datasets
    rob_alphabets = {
        'Rob01': string.ascii_uppercase[:4],  # A-D
        'Rob02': string.ascii_uppercase[:8],  # A-H
        'Rob03': string.ascii_uppercase[:12],  # A-L
        'Rob04': string.ascii_uppercase[:16],  # A-P
    }

    data_dir = os.path.join('expr5', 'datasets')
    query_dir = os.path.join('expr5', 'queries')
    os.makedirs(data_dir, exist_ok=True)
    os.makedirs(query_dir, exist_ok=True)

    # Generate datasets for Rob01 to Rob04
    for rob_name, alphabet in rob_alphabets.items():
        dataset = generate_dataset_expr5(alphabet)
        save_dataset(dataset, os.path.join(data_dir, f'{rob_name}.txt'))

        # Generate 10%, 30%, and 50% samples and their query workloads
        for sample_pct in [0.1, 0.3, 0.5]:
            sample = sample_dataset(dataset, sample_pct)
            queries = generate_query_workload5(sample, int(len(dataset) * sample_pct), 3, 8)
            save_queries(queries, os.path.join(query_dir, f'{rob_name}_queries_{int(sample_pct*100)}pct.txt'))
        # Generate 2% test query set
        test_sample = sample_dataset(dataset, 0.02)
        test_queries = generate_query_workload(sample, int(len(dataset) * 0.02), 3, 8)
        save_queries(test_queries, os.path.join(query_dir, f'{rob_name}_test_queries_2pct.txt'))

# generate_expr5()


# # Comprehensive Configurable Synthetic Dataset Generator
# 
# This section provides a unified, configurable approach to generate synthetic datasets with varying:
# - Dataset sizes
# - Query set sizes  
# - Average selectivities
# - Alphabet sizes
# - String length distributions

# In[ ]:


class ConfigurableSyntheticGenerator:
    """
    A comprehensive synthetic dataset generator with configurable parameters.

    Parameters:
    - alphabet_sizes: List of alphabet sizes (e.g., [4, 8, 12, 16])
    - dataset_sizes: List of dataset sizes (e.g., [10000, 50000, 100000])
    - query_set_sizes: List of query set sizes (e.g., [100, 500, 1000])
    - selectivity_targets: List of target selectivities (e.g., [0.01, 0.05, 0.1])
    - string_length_params: Dict with 'min', 'max', 'mean' for string lengths
    """

    def __init__(self, config):
        self.config = config
        self.alphabets = {}
        self.dataset_cache = {}  # Cache for datasets
        self.compiled_patterns = {}  # Cache for compiled regex patterns
        self.max_workers = min(mp.cpu_count(), 8)  # Limit threads for memory efficiency
        self.lock = Lock()
        self._initialize_alphabets()
        print(f"Initialized generator with {self.max_workers} worker threads")

    def _initialize_alphabets(self):
        """Initialize alphabets of different sizes."""
        full_alphabet = string.ascii_uppercase + string.digits + string.punctuation
        for size in self.config['alphabet_sizes']:
            self.alphabets[size] = full_alphabet[:size]

    def generate_string(self, alphabet, length_params):
        """Generate a single string with given alphabet and length parameters."""
        if 'fixed' in length_params:
            length = length_params['fixed']
        else:
            # Use normal distribution for length
            length = max(1, int(np.random.normal(
                length_params.get('mean', 100),
                length_params.get('std', 20)
            )))
            length = min(max(length, length_params.get('min', 10)), 
                        length_params.get('max', 500))

        return ''.join(random.choices(alphabet, k=length))

    def generate_string_batch(self, alphabet, length_params, batch_size):
        """Generate a batch of strings efficiently."""
        strings = []
        
        if 'fixed' in length_params:
            length = length_params['fixed']
            # Pre-generate all random choices for efficiency
            all_choices = np.random.choice(list(alphabet), size=(batch_size, length))
            strings = [''.join(choices) for choices in all_choices]
        else:
            # Pre-generate lengths using vectorized operations
            lengths = np.random.normal(
                length_params.get('mean', 100),
                length_params.get('std', 20),
                batch_size
            ).astype(int)
            lengths = np.clip(lengths, 
                            length_params.get('min', 10), 
                            length_params.get('max', 500))
            
            # Generate strings in batch
            for length in lengths:
                if length > 0:
                    string_item = ''.join(random.choices(alphabet, k=length))
                    strings.append(string_item)
        
        return strings

    def generate_dataset_chunk(self, args):
        """Generate a chunk of dataset in parallel."""
        alphabet, length_params, chunk_size, seed_offset = args
        
        # Set unique random seed for this chunk
        np.random.seed(15213 + seed_offset)
        random.seed(53711 + seed_offset)
        
        return self.generate_string_batch(alphabet, length_params, chunk_size)

    def generate_dataset(self, alphabet_size, dataset_size, string_length_params):
        """Generate a dataset with specified parameters using multi-threading."""
        # Check cache first
        cache_key = (alphabet_size, dataset_size, str(string_length_params))
        if cache_key in self.dataset_cache:
            print(f"  Using cached dataset for alphabet={alphabet_size}, size={dataset_size}")
            return self.dataset_cache[cache_key]

        start_time = time.time()
        alphabet = self.alphabets[alphabet_size]
        
        # Determine optimal chunk size based on dataset size
        chunk_size = max(100, min(dataset_size // self.max_workers, 10000))
        num_chunks = (dataset_size + chunk_size - 1) // chunk_size
        
        print(f"  Generating dataset with {num_chunks} chunks of ~{chunk_size} strings each")
        
        # Prepare arguments for parallel processing
        chunk_args = []
        remaining = dataset_size
        for i in range(num_chunks):
            current_chunk_size = min(chunk_size, remaining)
            chunk_args.append((alphabet, string_length_params, current_chunk_size, i))
            remaining -= current_chunk_size
        
        # Generate dataset chunks in parallel
        dataset = []
        with ThreadPoolExecutor(max_workers=self.max_workers) as executor:
            future_to_chunk = {executor.submit(self.generate_dataset_chunk, args): i 
                             for i, args in enumerate(chunk_args)}
            
            for future in as_completed(future_to_chunk):
                chunk_strings = future.result()
                dataset.extend(chunk_strings)
        
        # Cache the dataset
        with self.lock:
            self.dataset_cache[cache_key] = dataset
        
        generation_time = time.time() - start_time
        print(f"  Generated {len(dataset)} strings in {generation_time:.2f}s")
        
        return dataset

    def calculate_selectivity(self, query_pattern, dataset):
        """Calculate the actual selectivity of a query pattern in the dataset with caching."""
        # Use cached compiled pattern if available
        if query_pattern not in self.compiled_patterns:
            try:
                self.compiled_patterns[query_pattern] = re.compile(query_pattern)
            except re.error:
                return 0.0
        
        compiled_pattern = self.compiled_patterns[query_pattern]
        
        # Use batch processing for large datasets
        if len(dataset) > 1000:
            # Process in batches to reduce memory usage
            batch_size = 1000
            matches = 0
            for i in range(0, len(dataset), batch_size):
                batch = dataset[i:i + batch_size]
                matches += sum(1 for string in batch if compiled_pattern.search(string))
            return matches / len(dataset)
        else:
            matches = sum(1 for string in dataset if compiled_pattern.search(string))
            return matches / len(dataset)

    def generate_query_with_target_selectivity(self, dataset, target_selectivity, 
                                             max_attempts=50):
        """Generate a query that approximately matches the target selectivity."""
        alphabet = list(set(''.join(dataset[:min(100, len(dataset))])))  # Sample for alphabet
        query = ""
        actual_selectivity = 0.0
        best_query = ""
        best_selectivity = 0.0

        for attempt in range(max_attempts):
            # Generate query components with varying complexity based on target selectivity
            if target_selectivity > 0.1:
                # Simple patterns for high selectivity
                chars1 = ''.join(random.choices(alphabet, k=random.randint(2, 3)))
                gap = random.randint(1, 10)
                query = f"{chars1}(.{{0,{gap}}})"
            elif target_selectivity > 0.05:
                # Medium complexity
                chars1 = ''.join(random.choices(alphabet, k=random.randint(3, 4)))
                chars2 = ''.join(random.choices(alphabet, k=random.randint(2, 3)))
                gap = random.randint(1, 15)
                query = f"{chars1}(.{{0,{gap}}}){chars2}"
            else:
                # Complex patterns for low selectivity
                chars1 = ''.join(random.choices(alphabet, k=random.randint(4, 6)))
                chars2 = ''.join(random.choices(alphabet, k=random.randint(3, 5)))
                chars3 = ''.join(random.choices(alphabet, k=random.randint(2, 4)))
                gap1 = random.randint(1, 20)
                gap2 = random.randint(1, 20)
                query = f"{chars1}(.{{0,{gap1}}}){chars2}(.{{0,{gap2}}}){chars3}"

            actual_selectivity = self.calculate_selectivity(query, dataset)

            # Track best attempt
            if abs(actual_selectivity - target_selectivity) < abs(best_selectivity - target_selectivity):
                best_query = query
                best_selectivity = actual_selectivity

            # Accept if within reasonable range of target
            tolerance = max(target_selectivity * 0.3, 0.01)  # Adaptive tolerance
            if abs(actual_selectivity - target_selectivity) <= tolerance:
                return query, actual_selectivity

        # If no good match found, return best attempt
        return best_query, best_selectivity

    def generate_query_set_parallel(self, args):
        """Generate a subset of queries in parallel."""
        dataset, target_selectivity, num_queries, seed_offset = args
        
        # Set unique random seed for this thread
        np.random.seed(15213 + seed_offset)
        random.seed(53711 + seed_offset)
        
        queries = []
        actual_selectivities = []
        
        for _ in range(num_queries):
            query, actual_sel = self.generate_query_with_target_selectivity(
                dataset, target_selectivity
            )
            queries.append(query)
            actual_selectivities.append(actual_sel)
        
        return queries, actual_selectivities

    def generate_query_set(self, dataset, query_set_size, target_selectivity):
        """Generate a set of queries with approximately the target selectivity using parallel processing."""
        start_time = time.time()
        
        # For small query sets, use single-threaded approach
        if query_set_size <= 20:
            queries = []
            actual_selectivities = []
            for _ in range(query_set_size):
                query, actual_sel = self.generate_query_with_target_selectivity(
                    dataset, target_selectivity
                )
                queries.append(query)
                actual_selectivities.append(actual_sel)
            
            query_time = time.time() - start_time
            print(f"    Generated {len(queries)} queries in {query_time:.2f}s (single-threaded)")
            return queries, actual_selectivities
        
        # For larger query sets, use parallel processing
        queries_per_worker = max(1, query_set_size // self.max_workers)
        worker_args = []
        remaining = query_set_size
        
        for i in range(self.max_workers):
            if remaining <= 0:
                break
            num_queries = min(queries_per_worker, remaining)
            worker_args.append((dataset, target_selectivity, num_queries, i))
            remaining -= num_queries
        
        all_queries = []
        all_selectivities = []
        
        with ThreadPoolExecutor(max_workers=self.max_workers) as executor:
            future_to_worker = {executor.submit(self.generate_query_set_parallel, args): i 
                              for i, args in enumerate(worker_args)}
            
            for future in as_completed(future_to_worker):
                queries, selectivities = future.result()
                all_queries.extend(queries)
                all_selectivities.extend(selectivities)
        
        query_time = time.time() - start_time
        print(f"    Generated {len(all_queries)} queries in {query_time:.2f}s (multi-threaded)")
        return all_queries, all_selectivities

    def generate_comprehensive_benchmark(self, output_dir):
        """Generate a comprehensive benchmark suite with optimizations."""
        os.makedirs(output_dir, exist_ok=True)

        results = []
        files_generated = 0
        files_skipped = 0
        total_configs = (len(self.config['alphabet_sizes']) * 
                        len(self.config['dataset_sizes']) * 
                        len(self.config['selectivity_targets']) * 
                        len(self.config['query_set_sizes']))
        
        print(f"Starting generation of {total_configs} total configurations...")
        start_time = time.time()

        for alphabet_size in self.config['alphabet_sizes']:
            for dataset_size in self.config['dataset_sizes']:
                for target_selectivity in self.config['selectivity_targets']:

                    print(f"\nGenerating: alphabet={alphabet_size}, "
                          f"dataset_size={dataset_size}, "
                          f"selectivity={target_selectivity}")

                    # Generate or retrieve cached dataset
                    dataset = self.generate_dataset(
                        alphabet_size, 
                        dataset_size, 
                        self.config['string_length_params']
                    )

                    # Generate query sets of different sizes
                    for query_set_size in self.config['query_set_sizes']:
                        # Create unique identifier
                        identifier = f"alph{alphabet_size}_data{dataset_size}_qs{query_set_size}_sel{target_selectivity:.3f}"

                        # Define file paths
                        dataset_file = os.path.join(output_dir, f"dataset_{identifier}.txt")
                        query_file = os.path.join(output_dir, f"queries_{identifier}.txt")
                        
                        # Check if files already exist
                        if os.path.exists(dataset_file) and os.path.exists(query_file):
                            print(f"  Skipping {identifier} - files already exist")
                            files_skipped += 1
                            
                            # Still record metadata for existing files
                            results.append({
                                'alphabet_size': alphabet_size,
                                'dataset_size': dataset_size,
                                'query_set_size': query_set_size,
                                'target_selectivity': target_selectivity,
                                'actual_avg_selectivity': target_selectivity,  # Approximate for existing files
                                'dataset_file': dataset_file,
                                'query_file': query_file,
                                'identifier': identifier
                            })
                            continue
                        
                        print(f"  Generating queries for {identifier}")
                        files_generated += 1
                        
                        queries, actual_sels = self.generate_query_set(
                            dataset, query_set_size, target_selectivity
                        )

                        # Save dataset (only if it doesn't exist)
                        if not os.path.exists(dataset_file):
                            print(f"    Saving dataset ({len(dataset)} strings)")
                            with open(dataset_file, 'w') as f:
                                for line in dataset:
                                    f.write(line + '\n')

                        # Save queries (only if it doesn't exist)
                        if not os.path.exists(query_file):
                            print(f"    Saving queries ({len(queries)} queries)")
                            with open(query_file, 'w') as f:
                                for query in queries:
                                    f.write(query + '\n')

                        # Record metadata
                        avg_actual_selectivity = np.mean(actual_sels)
                        results.append({
                            'alphabet_size': alphabet_size,
                            'dataset_size': dataset_size,
                            'query_set_size': query_set_size,
                            'target_selectivity': target_selectivity,
                            'actual_avg_selectivity': avg_actual_selectivity,
                            'dataset_file': dataset_file,
                            'query_file': query_file,
                            'identifier': identifier
                        })

        # Print generation summary
        total_time = time.time() - start_time
        print(f"\nGeneration Summary:")
        print(f"  Files generated: {files_generated}")
        print(f"  Files skipped: {files_skipped}")
        print(f"  Total configurations: {len(results)}")
        print(f"  Unique datasets cached: {len(self.dataset_cache)}")
        print(f"  Total time: {total_time:.2f}s")
        print(f"  Average time per new config: {total_time/max(files_generated, 1):.2f}s")

        # Save metadata
        metadata_file = os.path.join(output_dir, 'benchmark_metadata.pkl')
        with open(metadata_file, 'wb') as f:
            pickle.dump(results, f)

        # Save human-readable summary with performance metrics
        summary_file = os.path.join(output_dir, 'benchmark_summary.txt')
        with open(summary_file, 'w') as f:
            f.write("Synthetic Benchmark Summary\n")
            f.write("=" * 50 + "\n\n")
            f.write(f"Generation Statistics:\n")
            f.write(f"  Files generated: {files_generated}\n")
            f.write(f"  Files skipped: {files_skipped}\n")
            f.write(f"  Total configurations: {len(results)}\n")
            f.write(f"  Unique datasets cached: {len(self.dataset_cache)}\n")
            f.write(f"  Total generation time: {total_time:.2f}s\n")
            f.write(f"  Thread workers used: {self.max_workers}\n\n")
            
            for result in results:
                f.write(f"Configuration: {result['identifier']}\n")
                f.write(f"  Alphabet Size: {result['alphabet_size']}\n")
                f.write(f"  Dataset Size: {result['dataset_size']}\n")
                f.write(f"  Query Set Size: {result['query_set_size']}\n")
                f.write(f"  Target Selectivity: {result['target_selectivity']:.3f}\n")
                f.write(f"  Actual Avg Selectivity: {result['actual_avg_selectivity']:.3f}\n")
                f.write(f"  Dataset File: {result['dataset_file']}\n")
                f.write(f"  Query File: {result['query_file']}\n")
                f.write("\n")

        print(f"\nBenchmark generation completed!")
        print(f"Total configurations: {len(results)} in {output_dir}")
        return results

    def clear_cache(self):
        """Clear cached data to free memory."""
        self.dataset_cache.clear()
        self.compiled_patterns.clear()
        print("Cleared all caches")

    def get_memory_usage(self):
        """Get approximate memory usage of cached data."""
        dataset_memory = sum(len(str(dataset)) for dataset in self.dataset_cache.values())
        pattern_memory = len(self.compiled_patterns) * 100  # Rough estimate
        return {
            'dataset_cache_items': len(self.dataset_cache),
            'pattern_cache_items': len(self.compiled_patterns),
            'estimated_dataset_memory_bytes': dataset_memory,
            'estimated_pattern_memory_bytes': pattern_memory
        }


# In[ ]:


# Example configurations for different experimental scenarios

# Configuration 1: Small-scale testing
small_scale_config = {
    'alphabet_sizes': [4, 8, 16],
    'dataset_sizes': [1000, 5000, 10000],
    'query_set_sizes': [50, 100, 200],
    'selectivity_targets': [0.01, 0.05, 0.1, 0.2],
    'string_length_params': {
        'mean': 100,
        'std': 20,
        'min': 20,
        'max': 200
    }
}

# Configuration 2: Medium-scale evaluation  
medium_scale_config = {
    'alphabet_sizes': [8, 16, 26, 32],
    'dataset_sizes': [10000, 50000, 100000],
    'query_set_sizes': [100, 500, 1000],
    'selectivity_targets': [0.005, 0.01, 0.02, 0.05, 0.1],
    'string_length_params': {
        'mean': 150,
        'std': 30,
        'min': 50,
        'max': 300
    }
}

# Configuration 3: Large-scale benchmarking
large_scale_config = {
    'alphabet_sizes': [16, 26, 52, 64],  # uppercase, upper+lower, alphanumeric, extended
    'dataset_sizes': [50000, 100000, 200000, 500000],
    'query_set_sizes': [500, 1000, 2000, 5000],
    'selectivity_targets': [0.001, 0.005, 0.01, 0.02, 0.05, 0.1, 0.2],
    'string_length_params': {
        'mean': 200,
        'std': 50,
        'min': 50,
        'max': 500
    }
}

# Configuration 4: Fixed-length strings (similar to existing methods)
fixed_length_config = {
    'alphabet_sizes': [4, 8, 12, 16, 26],
    'dataset_sizes': [20000, 40000, 60000, 80000, 100000],
    'query_set_sizes': [100, 500, 1000, 2000],
    'selectivity_targets': [0.01, 0.05, 0.1, 0.15, 0.2],
    'string_length_params': {
        'fixed': 450  # Fixed length like in expr2
    }
}


# In[ ]:


def generate_configurable_benchmarks():
    """Generate benchmarks using the configurable generator."""

    # Choose configuration based on desired scale
    config = large_scale_config  # Change this to test different scales

    # Create generator
    generator = ConfigurableSyntheticGenerator(config)

    # Generate comprehensive benchmark
    output_dir = os.path.join(DATA_DIR, 'configurable_synthetic')
    results = generator.generate_comprehensive_benchmark(output_dir)

    # Print summary statistics
    print(f"\nGenerated {len(results)} benchmark configurations")
    print(f"Alphabet sizes: {sorted(set(r['alphabet_size'] for r in results))}")
    print(f"Dataset sizes: {sorted(set(r['dataset_size'] for r in results))}")
    print(f"Query set sizes: {sorted(set(r['query_set_size'] for r in results))}")
    print(f"Target selectivities: {sorted(set(r['target_selectivity'] for r in results))}")

    # Analyze selectivity accuracy
    selectivity_errors = [
        abs(r['actual_avg_selectivity'] - r['target_selectivity']) 
        for r in results
    ]
    print(f"\nSelectivity targeting accuracy:")
    print(f"Mean absolute error: {np.mean(selectivity_errors):.4f}")
    print(f"Max absolute error: {np.max(selectivity_errors):.4f}")
    print(f"90th percentile error: {np.percentile(selectivity_errors, 90):.4f}")

    return results

def generate_large_scale_benchmarks():
    """Generate large-scale benchmarks for comprehensive evaluation."""

    # Use the large-scale configuration
    config = large_scale_config

    # Create generator
    generator = ConfigurableSyntheticGenerator(config)

    # Generate comprehensive benchmark
    output_dir = os.path.join(DATA_DIR, 'large_scale_synthetic')
    results = generator.generate_comprehensive_benchmark(output_dir)

    # Print summary statistics
    print(f"\nGenerated {len(results)} large-scale benchmark configurations")
    print(f"Total configurations: {len(results)}")
    print(f"Alphabet sizes: {sorted(set(r['alphabet_size'] for r in results))}")
    print(f"Dataset sizes: {sorted(set(r['dataset_size'] for r in results))}")
    print(f"Query set sizes: {sorted(set(r['query_set_size'] for r in results))}")
    print(f"Target selectivities: {sorted(set(r['target_selectivity'] for r in results))}")

    # Analyze selectivity accuracy
    selectivity_errors = [
        abs(r['actual_avg_selectivity'] - r['target_selectivity']) 
        for r in results
    ]
    print(f"\nSelectivity targeting accuracy:")
    print(f"Mean absolute error: {np.mean(selectivity_errors):.4f}")
    print(f"Max absolute error: {np.max(selectivity_errors):.4f}")
    print(f"90th percentile error: {np.percentile(selectivity_errors, 90):.4f}")

    return results

# Example: Generate a small test set
def generate_test_benchmark():
    """Generate a small test benchmark for validation."""
    test_config = {
        'alphabet_sizes': [4, 8],
        'dataset_sizes': [1000, 2000],
        'query_set_sizes': [50, 100],
        'selectivity_targets': [0.05, 0.1],
        'string_length_params': {
            'mean': 80,
            'std': 15,
            'min': 30,
            'max': 150
        }
    }

    generator = ConfigurableSyntheticGenerator(test_config)
    output_dir = os.path.join(DATA_DIR, 'test_synthetic')
    results = generator.generate_comprehensive_benchmark(output_dir)

    print(f"Generated test benchmark with {len(results)} configurations in {output_dir}")
    return results


# In[ ]:


# Uncomment to generate test benchmark
# test_results = generate_test_benchmark()

# Uncomment to generate full configurable benchmark (uses large_scale_config now)
# results = generate_configurable_benchmarks()

# Uncomment to generate large-scale benchmarks specifically
# large_results = generate_large_scale_benchmarks()


# In[ ]:


def generate_synthetic_data_from_script(config_name='large_scale_config', output_dir='./configurable_synthetic'):
    """
    Convenient function to generate synthetic data from external scripts.

    Args:
        config_name: Name of configuration to use ('small_scale_config', 'medium_scale_config', 
                    'large_scale_config', 'fixed_length_config')
        output_dir: Directory to save the generated data

    Returns:
        List of generated benchmark results
    """

    # Configuration mapping
    configs = {
        'small_scale_config': small_scale_config,
        'medium_scale_config': medium_scale_config, 
        'large_scale_config': large_scale_config,
        'fixed_length_config': fixed_length_config
    }

    if config_name not in configs:
        raise ValueError(f"Unknown config: {config_name}. Available: {list(configs.keys())}")

    config = configs[config_name]

    print(f"Generating synthetic data with {config_name}")
    print(f"Output directory: {output_dir}")

    # Create generator and run
    generator = ConfigurableSyntheticGenerator(config)
    results = generator.generate_comprehensive_benchmark(output_dir)

    print(f"Successfully generated {len(results)} benchmark configurations")
    return results

# Example usage:
# results = generate_synthetic_data_from_script('small_scale_config', './test_synthetic')
results = generate_synthetic_data_from_script('large_scale_config', './configurable_synthetic')


# In[ ]:


def analyze_synthetic_benchmark(results_file):
    """Analyze the generated synthetic benchmark."""
    with open(results_file, 'rb') as f:
        results = pickle.load(f)

    # Group by different parameters
    by_alphabet = defaultdict(list)
    by_dataset_size = defaultdict(list)
    by_query_size = defaultdict(list)
    by_selectivity = defaultdict(list)

    for result in results:
        by_alphabet[result['alphabet_size']].append(result)
        by_dataset_size[result['dataset_size']].append(result)
        by_query_size[result['query_set_size']].append(result)
        by_selectivity[result['target_selectivity']].append(result)

    print("Analysis of Synthetic Benchmark")
    print("=" * 50)

    print(f"\nBy Alphabet Size:")
    for alph_size, configs in sorted(by_alphabet.items()):
        selectivities = [c['actual_avg_selectivity'] for c in configs]
        print(f"  {alph_size}: {len(configs)} configs, "
              f"selectivity range [{min(selectivities):.4f}, {max(selectivities):.4f}]")

    print(f"\nBy Dataset Size:")
    for data_size, configs in sorted(by_dataset_size.items()):
        selectivities = [c['actual_avg_selectivity'] for c in configs]
        print(f"  {data_size}: {len(configs)} configs, "
              f"selectivity range [{min(selectivities):.4f}, {max(selectivities):.4f}]")

    print(f"\nBy Query Set Size:")
    for query_size, configs in sorted(by_query_size.items()):
        selectivities = [c['actual_avg_selectivity'] for c in configs]
        print(f"  {query_size}: {len(configs)} configs, "
              f"selectivity range [{min(selectivities):.4f}, {max(selectivities):.4f}]")

    print(f"\nSelectivity Target vs Actual:")
    for target_sel, configs in sorted(by_selectivity.items()):
        actual_sels = [c['actual_avg_selectivity'] for c in configs]
        mean_actual = np.mean(actual_sels)
        std_actual = np.std(actual_sels)
        print(f"  Target {target_sel:.3f}: "
              f"Actual {mean_actual:.4f} ± {std_actual:.4f} "
              f"(error: {abs(target_sel - mean_actual):.4f})")


