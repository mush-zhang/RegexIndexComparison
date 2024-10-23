#!/usr/bin/env python
# coding: utf-8

# In[1]:


import os
import sys
import numpy as np
import string
import pickle

from collections import defaultdict
from multiprocessing import Process, Manager, cpu_count


# In[2]:


dir_name = '../data/synthetic/'
result_dir_large = os.path.join(dir_name, 'large')
result_dir_small = os.path.join(dir_name, 'small')


# In[ ]:


DATA_NUM = 400000
QUERY_NUM = 230
MEAN = [800, 1300, 1900, 200]
SD = [500, 300, 200, 100]


# ## Get all prefix-free multi-grams

# In[ ]:


# Parameters
global MIN_LENGTH
MIN_LENGTH = 3
global MAX_LENGTH
MAX_LENGTH = 3
global ALPHABET
ALPHABET = string.ascii_uppercase


# In[6]:


class TrieNode:
    """A node in the Trie."""
    def __init__(self):
        self.children = defaultdict(TrieNode)
        self.is_end_of_string = False

class Trie:
    """Trie data structure to store strings and check for prefixes."""
    def __init__(self):
        self.root = TrieNode()

    def insert(self, s):
        """Insert a string into the Trie."""
        node = self.root
        for char in s:
            node = node.children[char]
        node.is_end_of_string = True

    def is_prefix(self, s):
        """Check if 's' or any of its prefixes exist in the Trie."""
        node = self.root
        for char in s:
            if node.is_end_of_string:
                return True  # 's' is a prefix of an existing string
            if char not in node.children:
                return False  # No conflict
            node = node.children[char]
        return node.is_end_of_string  # Exact match check

# def generate_prefix_free_set(start_letter, MIN_LENGTH, MAX_LENGTH, shared_set, lock):
def generate_prefix_free_set(start_letter, shared_set, lock):
    """Generate a prefix-free set starting with a specific letter."""
    trie = Trie()

    def backtrack(current_string):
        """Recursively generate prefix-free strings."""
        if MIN_LENGTH <= len(current_string) <= MAX_LENGTH:
            if not trie.is_prefix(current_string):
                # Add to shared set in a thread-safe way
                with lock:
                    shared_set.append(current_string)
                trie.insert(current_string)

        if len(current_string) >= MAX_LENGTH:
            return

        for char in ALPHABET:
            backtrack(current_string + char)

    # Start the generation with the provided start letter
    backtrack(start_letter)


# In[ ]:


gram_fn = f'multigrams_{MIN_LENGTH}-{MAX_LENGTH}.pkl'

if os.path.exists(gram_fn):
    with open(gram_fn, 'rb') as f:
        prefix_free_strings = pickle.load(f)
    print(f"Loaded {len(prefix_free_strings)} prefix-free strings.")
else:
    # Shared set and lock for thread-safe access
    manager = Manager()
    shared_set = manager.list()  # Use a shared list to store results
    lock = manager.Lock()

    # Create processes for parallel generation
    processes = []
    for i in range(13):  # 13 threads
        start_letter = ALPHABET[i]
        p = Process(target=generate_prefix_free_set,
                    args=(start_letter, shared_set, lock))
        processes.append(p)

    # Start all processes
    for p in processes:
        p.start()

    # Wait for all processes to complete
    for p in processes:
        p.join()

    # Convert the shared list to a set and print the results
    prefix_free_strings = set(shared_set)

    with open(gram_fn, 'wb') as f:
        pickle.dump(prefix_free_strings, f)
    print(f"Generated {len(prefix_free_strings)} prefix-free strings.")


# ## Assign frequencies to each multigrams

# In[ ]:




