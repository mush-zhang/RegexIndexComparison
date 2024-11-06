#!/usr/bin/env python
# coding: utf-8

# In[ ]:


import os


# # In[ ]:


# DATA_ROOT = '.'


# # ## Webpages

# # In[ ]:


# dataset_dir = os.path.join(DATA_ROOT, 'webpages/processed')


# # In[ ]:


# char_set = set()
# line_count = 0
# ave_line_len = 0
# for fn in os.listdir(dataset_dir):
#     with open(os.path.join(dataset_dir, fn), 'r', encoding='latin') as file:
#         curr_line = file.read()
#         if len(curr_line) < 3:
#             continue
#         ave_line_len = (line_count * ave_line_len + len(curr_line)) / (line_count + 1)
#         char_set = char_set.union(set(curr_line))
#         line_count += 1


# # In[ ]:


# print('Webpages')
# print(f'\t\tAlphabet Size: {len(char_set)}')
# print(f'\t\tDataset Size: {line_count}')
# print(f'\t\tAverage Num Chars in Dataset: {ave_line_len}')


# # ## DBLP

# # In[ ]:


# dataset_fn = os.path.join(DATA_ROOT, 'dblp/small/authors.txt')


# # In[ ]:


# with open(dataset_fn, 'r') as file:
#     dataset = file.readlines()

# char_set = set()
# line_count = 0
# ave_line_len = 0
# for line in dataset:
#     line = line.rstrip()
#     if len(line) < 3:
#         continue
#     ave_line_len = (line_count * ave_line_len + len(line)) / (line_count + 1)
#     char_set = char_set.union(set(line))
#     line_count += 1


# # In[ ]:


# print('DBLP')
# print(f'\t\tAlphabet Size: {len(char_set)}')
# print(f'\t\tDataset Size: {line_count}')
# print(f'\t\tAverage Num Chars in Dataset: {ave_line_len}')


# # ## Prosite

# # In[ ]:


# dataset_dir = os.path.join(DATA_ROOT, 'protein/sequences')
# query_fn = os.path.join(DATA_ROOT, 'protein/prosites.txt')


# # In[ ]:


# char_set = set()
# line_count = 0
# ave_line_len = 0
# for fn in os.listdir(dataset_dir):
#     with open(os.path.join(dataset_dir, fn), 'r') as file:
#         curr_dataset = file.readlines()
#     for line in curr_dataset:
#         line = line.rstrip()
#         if len(line) < 3:
#             continue
#         ave_line_len = (line_count * ave_line_len + len(line)) / (line_count + 1)
#         char_set = char_set.union(set(line))
#         line_count += 1


# # In[ ]:


# with open(query_fn, 'r') as file:
#     query_set = file.readlines()


# # In[ ]:


# print('Protein')
# print(f'\t\tAlphabet Size: {len(char_set)}')
# print(f'\t\tDataset Size: {line_count}')
# print(f'\t\tQuery Set Size: {len(query_set)}')
# print(f'\t\tAverage Num Chars in Dataset: {ave_line_len}')


# # ## US-Accident

# # In[ ]:


# dataset_fn = os.path.join(DATA_ROOT, 'US_Accidents_Dec21_updated.csv')


# # In[ ]:


# with open(dataset_fn, 'r') as file:
#     dataset = file.readlines()

# char_set = set()
# line_count = 0
# ave_line_len = 0
# for line in dataset:
#     line = line.rstrip()
#     if len(line) < 3:
#         continue
#     ave_line_len = (line_count * ave_line_len + len(line)) / (line_count + 1)
#     char_set = char_set.union(set(line))
#     line_count += 1


# # In[ ]:


# print('US-Accident')
# print(f'\t\tAlphabet Size: {len(char_set)}')
# print(f'\t\tDataset Size: {line_count}')
# print(f'\t\tAverage Num Chars in Dataset: {ave_line_len}')


# # ## Sql-Server

# # In[ ]:


# dataset_dir = os.path.join(DATA_ROOT, 'extracted')

# # In[ ]:


# char_set = set()
# line_count = 0
# ave_line_len = 0
# for fn in os.listdir(dataset_dir):
#     with open(os.path.join(dataset_dir, fn), 'r') as file:
#         curr_dataset = file.readlines()
#     for line in curr_dataset:
#         line = line.rstrip()
#         if len(line) < 3:
#             continue
#         ave_line_len = (line_count * ave_line_len + len(line)) / (line_count + 1)
#         char_set = char_set.union(set(line))
#         line_count += 1

# print('SQL-Server')
# print(f'\t\tAlphabet Size: {len(char_set)}')
# print(f'\t\tDataset Size: {line_count}')
# print(f'\t\tAverage Num Chars in Dataset: {ave_line_len}')

query_fn = os.path.join(DATA_ROOT, 'db-x_mid.txt')
with open(query_fn, 'r') as file:
    query_set = file.readlines()
print(f'\t\tQuery Set Mid Size: {len(query_set)}')

query_fn = os.path.join(DATA_ROOT, 'db-x_full.txt')
with open(query_fn, 'r', encoding='latin') as file:
    query_set = file.readlines()
print(f'\t\tQuery Set Full Size: {len(query_set)}')

query_fn = os.path.join(DATA_ROOT, 'regexes_dbx.txt')
with open(query_fn, 'r') as file:
    query_set = file.readlines()
print(f'\t\tQuery Set Default Size: {len(query_set)}')
