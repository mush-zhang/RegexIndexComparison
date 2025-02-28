#!/usr/bin/env python
# coding: utf-8

# In[1]:


import os
import sys
import numpy as np
import string
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


# In[ ]:




