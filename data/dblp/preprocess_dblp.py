#!/usr/bin/env python
# coding: utf-8

# In[1]:


import os
import sys
import numpy as np


# In[2]:


dir_name = '.'
raw_file = os.path.join(dir_name, 'DBLPOnlyCitationOct19.txt')


# In[3]:


with open(raw_file) as f:
    lines = f.read().splitlines() 


# In[4]:


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

# random select 305,798 tuples?
keep_idxs = np.random.choice(len(titles), size=305798, replace=False)
titles = np.array(titles)[keep_idxs]
authors = np.array(titles)[authors]


# In[5]:


with open(os.path.join(dir_name, 'authors.txt'), 'w') as out:
    out.writelines([author+'\n' for author in authors])


# In[6]:


with open(os.path.join(dir_name, 'titles.txt'), 'w') as out:
    out.writelines([title+'\n' for title in titles])


# ## Generate query workloads of size 1k, 2k, 3k, and 4k

# In[8]:


# get bag of author names
last_names = []
for author_list in authors:
    splitted = author_list.split(',')
    last_names += [name.split(' ')[-1] for name in splitted]


# In[12]:


for n in [1000, 2000, 3000, 4000]:
    print(f'random select {n} last names')
    keep_idxs = np.random.choice(len(last_names), size=n, replace=False)
    print('finish selection. writing to file')
    with open(os.path.join(dir_name, f'query{n}.txt'), 'w') as out:
        out.writelines([f'([^,]+) {last_names[idx]}\n' for idx in keep_idxs])


# In[ ]:




