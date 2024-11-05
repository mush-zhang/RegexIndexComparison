#!/usr/bin/env python
# coding: utf-8

# In[ ]:


import os


# In[ ]:


DATA_ROOT = '.'


# ## Webpages

# In[ ]:


dataset_dir = os.path.join(DATA_ROOT, 'webpages/processed')


# In[ ]:


char_set = {}
line_count = 0
ave_line_len = 0
for fn in os.listdir(dataset_dir):
    with open(os.path.join(dataset_dir, fn), 'r') as file:
        curr_line = file.read()
        ave_line_len = (line_count * ave_line_len + len(curr_line)) / (line_count + 1)
        char_set.join(set(curr_line))
        line_count += 1


# In[ ]:


print('Webpages')
print(f'\t\tAlphabet Size: {len(char_set)}')
print(f'\t\tDataset Size: {line_count}')
print(f'\t\tAverage Num Chars in Dataset: {ave_line_len}')


# ## DBLP

# In[ ]:


dataset_fn = os.path.join(DATA_DIR, 'dblp/small/authors.txt')


# In[ ]:


with open(dataset_fn, 'r') as file:
    dataset = file.readlines()

char_set = {}
line_count = 0
ave_line_len = 0
for line in dataset:
    line = line.rstrip()
    ave_line_len = (line_count * ave_line_len + len(line)) / (line_count + 1)
    char_set.join(set(line))
    line_count += 1


# In[ ]:


print('DBLP')
print(f'\t\tAlphabet Size: {len(char_set)}')
print(f'\t\tDataset Size: {line_count}')
print(f'\t\tAverage Num Chars in Dataset: {ave_line_len}')


# ## Prosite

# In[ ]:


dataset_dir = os.path.join(DATA_ROOT, 'protein/sequences')
query_fn = os.path.join(DATA_ROOT, 'protein/prosites.txt')


# In[ ]:


char_set = {}
line_count = 0
ave_line_len = 0
for fn in os.listdir(dataset_dir):
    with open(os.path.join(dataset_dir, fn), 'r') as file:
        curr_dataset = file.readlines()
    for line in curr_dataset:
        line = line.rstrip()
        ave_line_len = (line_count * ave_line_len + len(line)) / (line_count + 1)
        char_set.join(set(line))
        line_count += 1


# In[ ]:


with open(os.path.join(query_fn, fn), 'r') as file:
    query_set = file.readlines()


# In[ ]:


print('Protein')
print(f'\t\tAlphabet Size: {len(char_set)}')
print(f'\t\tDataset Size: {line_count}')
print(f'\t\tQuery Set Size: {len(query_set)}')
print(f'\t\tAverage Num Chars in Dataset: {ave_line_len}')


# ## US-Accident

# In[ ]:


dataset_fn = os.path.join(DATA_ROOT, 'US_Accidents_Dec21_updated.csv')


# In[ ]:


with open(dataset_fn, 'r') as file:
    dataset = file.readlines()

char_set = {}
line_count = 0
ave_line_len = 0
for line in dataset:
    line = line.rstrip()
    ave_line_len = (line_count * ave_line_len + len(line)) / (line_count + 1)
    char_set.join(set(line))
    line_count += 1


# In[ ]:


print('US-Accident')
print(f'\t\tAlphabet Size: {len(char_set)}')
print(f'\t\tDataset Size: {line_count}')
print(f'\t\tAverage Num Chars in Dataset: {ave_line_len}')


# ## Sql-Server

# In[ ]:


dataset_dir = os.path.join(DATA_ROOT, 'extraced')


# In[ ]:


char_set = {}
line_count = 0
ave_line_len = 0
for fn in os.listdir(dataset_dir):
    with open(os.path.join(dataset_dir, fn), 'r') as file:
        curr_dataset = file.readlines()
    for line in curr_dataset:
        line = line.rstrip()
        ave_line_len = (line_count * ave_line_len + len(line)) / (line_count + 1)
        char_set.join(set(line))
        line_count += 1

