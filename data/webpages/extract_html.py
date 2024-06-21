#!/usr/bin/env python
# coding: utf-8

# In[1]:


import os
import sys
import gzip
import random
import multiprocessing 

import re
import wget
from warcio.archiveiterator import ArchiveIterator


# In[2]:


FILE_UPPER_LIMIT = 700000
RAND_PROB = 0.002
MIN_EACH_REG = 10000
NUM_CONCURRENT = 12
PER_FILE_TIMEOUT=3600 # SECONDS


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

with open('regexes_webpages.txt', 'rb') as f:
    raw_regexes = f.read().splitlines() 
counts = multiprocessing.Array('i', range(len(raw_regexes)))
for i in range(len(raw_regexes)):
    raw_reg = raw_regexes[i]
    print(raw_reg)
    regexp = re.compile(raw_reg)
    regexes.append(regexp) 
    counts[i] = 0


# In[ ]:


def check_eligible(raw_regexes, regexes, shared_regex_counts, curr_content):
    # check if it has a regex match
    result = False
    for idx in range(len(regexes)):
        if shared_regex_counts[idx] >= MIN_EACH_REG:
            continue
        reg = regexes[idx]
        curr_result = reg.search(curr_content)
        if curr_result:
            result = True
            shared_regex_counts[idx] += 1
            if shared_regex_counts[idx] == 1:
                print(f'Find match for {raw_regexes[idx]} 1st time')

            if shared_regex_counts[idx] >= MIN_EACH_REG:
                print(f'NUMBER SATISFIED: {raw_regexes[idx]}')
            break

    if not result:
        result = random.random() < RAND_PROB
    return result


# In[ ]:


def extract_html(filename, raw_regexes, regexes, shared_regex_counts, shared_file_count):
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
                    if check_eligible(raw_regexes, regexes, shared_regex_counts, curr_content):
                        with open(os.path.join(processed_data_dir, f'{filename}_{idx}.txt'), 'wb') as file:
                            file.write(curr_content)
                        shared_file_count.Value += 1
                        if shared_file_count.Value % 1000 == 0:
                            print(f'Writing {shared_file_count.Value}-th file')
                        if shared_file_count.Value >= FILE_UPPER_LIMIT:
                            break
                    idx += 1


# In[ ]:


def task(p, raw_regexes, regexes, shared_regex_counts, shared_file_count):
    print(p)

    # download the warc file using path index
    wget.download(f'https://data.commoncrawl.org/{p}', temp_file_fd)

    filename = p.split('/')[-1]
    
    extract_html(filename, raw_regexes, regexes, shared_regex_counts, shared_file_count)
    os.remove(os.path.join(temp_file_fd, filename))


# In[ ]:


file_count = multiprocessing.Value('i', 0) #30731)
# processed 5 already with single-thread version of code
for i in range(0, len(paths), NUM_CONCURRENT):
    p = paths[i]
    processes = [multiprocessing.Process(target=task, args=[paths[j], raw_regexes, regexes, counts, file_count]) 
                for j in range(i, i+NUM_CONCURRENT)]

    for process in processes:
        process.start()

    for process in processes:
        process.join(PER_FILE_TIMEOUT)
    
    for process in processes:
        if process.is_alive():
            print(f'Try terminate pid={process.pid()}')
            process.terminate()
            print(f'Finish terminate pid={process.pid()}')
            process.join()
            # clean up un deleted file
            filename = p.split('/')[-1]
            if os.path.isfile(filename):
                os.remove(os.path.join(temp_file_fd, filename))
    if file_count.Value >= FILE_UPPER_LIMIT:
        break

