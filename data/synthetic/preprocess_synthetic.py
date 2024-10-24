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


dir_name = '../data/synthetic/'
result_dir_large = os.path.join(dir_name, 'large')
result_dir_small = os.path.join(dir_name, 'small')


# In[ ]:


random.seed(53711)


# In[ ]:


def generate_query_key():
    """Generate a random query key with length between 3 and 8 inclusively."""
    key_length = random.randint(3, 9)
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

def generate_frequencies(trigrams, mean, std_dev):
    """Generate target frequencies for the trigrams using a Normal distribution."""
    frequencies = np.random.normal(loc=mean, scale=std_dev, size=len(trigrams))
    # Ensure positive integer frequencies and avoid frequencies of 0
    frequencies = np.clip(np.abs(frequencies).astype(int), 1, None)
    return dict(zip(trigrams, frequencies))

directory_path = 'synthetic1'
if not os.path.isdir(directory_path):
    os.makedirs(directory_path)
    # Parameters
    dataset_size = 400_000  # Expected size of the dataset
    means = [100, 100, 100, 100]  # Mean frequency for all datasets
    std_devs = [100, 200, 300, 500]  # Four different standard deviations
    query_counts = [random.randint(227, 248) for _ in range(4)]
    # Generate all trigrams
    trigrams = generate_trigrams()

    # Shared manager object to store datasets
    manager = Manager()
    shared_dataset = manager.list([None] * 4)  # Store 4 datasets
    lock = Lock()

    # Create and start processes to generate multiple datasets in parallel
    processes = []
    for i in range(4):
        frequencies = generate_frequencies(trigrams, means[i], std_devs[i])
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


# In[ ]:


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

def plot_histogram(trigram_counter, dataset_index, output_dir):
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
    plt.xlabel('Number of strings a trigram appears in')
    plt.ylabel('Number of trigrams')
    plt.title(f'Trigram Distribution in Dataset {dataset_index + 1}')

    # Save the plot as a PDF
    output_file = os.path.join(output_dir, f'dataset_{dataset_index + 1}_histogram.pdf')
    plt.savefig(output_file)
    plt.close()

def main_analysis(datasets, trigrams):
    """
    Analyze all datasets and generate histogram plots.
    """
    output_dir = 'histogram_plots'
    os.makedirs(output_dir, exist_ok=True)  # Create output directory if it doesn't exist

    # Analyze each dataset and generate corresponding histograms
    for i, (dataset, _) in enumerate(datasets):
        trigram_counter = analyze_dataset(dataset, trigrams)
        plot_histogram(trigram_counter, i, output_dir)

    print(f'All histograms saved to: {output_dir}')

main_analysis(shared_dataset, trigrams)

