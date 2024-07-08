#!/usr/bin/env python
# coding: utf-8

# In[1]:


import os
import sys
import numpy as np


# In[11]:


dir_name = '.'
result_dir_large = os.path.join(dir_name, 'large')
result_dir_small = os.path.join(dir_name, 'small')
raw_file = os.path.join(dir_name, 'DBLPOnlyCitationOct19.txt')


# In[14]:


print(result_dir_large)


# In[15]:


if not os.path.exists(result_dir_large): 
    os.mkdir(result_dir_large)
if not os.path.exists(result_dir_small): 
    os.mkdir(result_dir_small)


# In[4]:


with open(raw_file) as f:
    lines = f.read().splitlines() 


# In[19]:


titles = []
authors = []
for line in lines:
    if line[:2] == '#*':
        assert(len(titles) == len(authors))
        titles.append(line[2:])
    elif line[:2] == '#@':
        assert(len(titles) == 1 + len(authors))
        line = line.strip()
        if len(line) == 2:
            del titles[-1]
        else:
            authors.append(line[2:])


# In[6]:


def write_dataset(dir_name, authors, titles):
    with open(os.path.join(dir_name, 'authors.txt'), 'w') as out:
        out.writelines([author+'\n' for author in authors])
    with open(os.path.join(dir_name, 'titles.txt'), 'w') as out:
        out.writelines([title+'\n' for title in titles])


# In[7]:


def write_query(dir_name, authors, titles):
    # get bag of author names
    last_names = []
    for author_list in authors:
        splitted = author_list.split(',')
        last_names += [name.split(' ')[-1] for name in splitted]
    for n in [1000, 2000, 3000, 4000]:
        print(f'random select {n} last names')
        keep_idxs = np.random.choice(len(last_names), size=n, replace=False)
        print('finish selection. writing to file')
        with open(os.path.join(dir_name, f'query{n}.txt'), 'w') as out:
            out.writelines([f'([^,]+) {last_names[idx]}\n' for idx in keep_idxs])


# ## Generate full workload

# In[16]:


write_dataset(result_dir_large, authors, titles)


# In[17]:


write_query(result_dir_large, authors, titles)


# ## Generate small workload

# In[20]:


keep_idxs = np.random.choice(len(titles), size=305798, replace=False)
titles = [titles[idx] for idx in keep_idxs]
authors = [authors[idx] for idx in keep_idxs]


# In[21]:


write_dataset(result_dir_small, authors, titles)


# In[22]:


write_query(result_dir_small, authors, titles)


# In[ ]:




