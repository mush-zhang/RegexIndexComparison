#!/usr/bin/env python
# coding: utf-8

# In[1]:


import os
import sys
import gzip
import random

import re
import wget
from warcio.archiveiterator import ArchiveIterator


# In[2]:


FILE_UPPER_LIMIT = 700000
RAND_PROB = 0.002
MIN_EACH_REG = 10000


# In[3]:


# create processed file folder if not exists
raw_data_dir = '.'
processed_data_dir = 'processed'
os.makedirs(processed_data_dir, exist_ok=True)


# In[ ]:


# download the index file
temp_file_parent_fd = '.' #'/mnt/'
temp_file_fd = os.path.join(temp_file_parent_fd, 'webtemp')
os.makedirs(temp_file_fd, exist_ok=True)
if not os.path.isfile(os.path.join(raw_data_dir, 'warc.paths.gz')):
    wget.download('https://data.commoncrawl.org/crawl-data/CC-MAIN-2013-48/warc.paths.gz', raw_data_dir)

with gzip.open('warc.paths.gz', 'rt') as f:
    paths = f.read().splitlines() 

print(paths[0])


# In[ ]:


# get all webpage regexes
regexes = []
counts = []
with open('regexes_webpages.txt', 'rb') as f:
    raw_regexes = f.read().splitlines() 
for raw_reg in raw_regexes:
    print(raw_reg)
    regexp = re.compile(raw_reg)
    regexes.append(regexp) 
    counts.append(0)


# In[ ]:


def check_eligible(curr_content):
    # check if it has a regex match
    result = False
    for idx in range(len(regexes)):
        reg = regexes[idx]
        curr_result = reg.search(curr_content)
        if curr_result:
            result = True
            counts[idx] += 1
            if counts[idx] == 1:
                print(f'Find match for {raw_regexes[idx]} 1st time')

            if counts[idx] >= MIN_EACH_REG:
                print(f'NUMBER SATISFIED: {raw_regexes[idx]}')
                del counts[idx]
                del raw_regexes[idx]
                del regexes[idx]
            break

    if not result:
        result = random.random() < RAND_PROB
    return result


# In[ ]:


def extract_html(filename, curr_count):
    idx = 0
    full_fname = os.path.join(temp_file_fd, filename)
    with open(full_fname, 'rb') as stream:
        for record in ArchiveIterator(stream):
            if record.rec_type == 'warcinfo':
                print('-------------------')
                print(record.raw_stream.read())
                print('\n-------------------')

            elif record.rec_type == 'response':
                header = record.http_headers.get_header('Content-Type')
                if header is not None and 'text/html' in header:
                    curr_content = record.content_stream().read()
                    
                    if check_eligible(curr_content):
                        with open(os.path.join(processed_data_dir, f'{filename}_{idx}.txt'), 'wb') as file:
                            file.write(curr_content)
                        curr_count += 1
                        if curr_count >= FILE_UPPER_LIMIT:
                            break
                    idx += 1
    return curr_count


# In[ ]:


finish = False

webpage_count = 0

for p in paths:
    if finish:
        break
    # download the warc file using path index
    wget.download(f'https://data.commoncrawl.org/{p}', temp_file_fd)
    
    filename = p.split('/')[-1]
    webpage_count = extract_html(filename, webpage_count)
    os.remove(os.path.join(temp_file_fd, filename))
    if webpage_count >= FILE_UPPER_LIMIT:
        break

