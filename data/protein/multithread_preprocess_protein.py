#!/usr/bin/env python
# coding: utf-8

# In[1]:


# standard library modules
import os, sys, errno, json, ssl, time
from urllib import request
from urllib.error import HTTPError
import multiprocessing as mp
import traceback
import math
import gc
import pickle


# In[2]:


DATA_DIR = '.'


# In[3]:


BASE_URL = "https://www.ebi.ac.uk/interpro/api"
global PFAM_INDI_URL_PREFIX
PFAM_INDI_URL_PREFIX = BASE_URL + '/protein/UniProt/entry/pfam/'
global PROTEIN_URL_PREFIX
PROTEIN_URL_PREFIX = BASE_URL + '/protein/uniprot/'
PFAM_LIST_URL = BASE_URL + '/entry/all/pfam'
global PROTEIN_LIST_FN
PROTEIN_LIST_FN = 'pfam_temp/protein_id_list'
global SEQ_FN 
SEQ_FN = 'sequences/seq'


PROSITE_LIST_URL = BASE_URL + '/entry/all/prosite'
PROSITE_URL_FORMAT = 'https://prosite.expasy.org/{}.txt' # fill in something starting with 'PS'


# In[4]:


global PFAM_UPPER_LIMIT
PFAM_UPPER_LIMIT = 100000

NUM_PROCESSES = 12


# In[5]:


def try_get_payload(curr_url, context, data_type='json'):
    last_page = False
    next_url = curr_url
    attempts = 0
    content_type = 'application/json' if data_type == 'json' else 'text/plain'
    while attempts < 3:
        try:
            req = request.Request(curr_url, headers={"Accept": content_type})
            res = request.urlopen(req, timeout=21, context=context)
            # If the API times out due a long running query
            if res.status == 408:
                sys.stderr.write("HTTP 408 error in response")
                # wait just over a minute
                time.sleep(21)
                # then continue this loop with the same URL
                attemps += 1
                continue
            elif res.status == 204:
                #no data so leave loop
                return True, None, None
            if data_type == 'json':
                payload = json.loads(res.read().decode())
                next_url = payload.get('next')
                if not next_url:
                    last_page = True
                return last_page, next_url, payload
            elif data_type == 'txt':
                return True, None, res.read().decode('utf-8')
            else:
                sys.stderr.write("Unknown data type: " + data_type)
                return last_page, None, None
        
        except HTTPError as e:
            sys.stderr.write(f"HTTP error: {e.code}" )
            attempts += 1
            time.sleep(21)
            continue
        except:
            attempts += 1
            time.sleep(21)
            print(attempts, curr_url)
            # print(traceback.format_exc())
    return last_page, next_url, None


# In[6]:


def get_family_full_list(start_url, upper_limit=21979):
    id_result = []
    
    #disable SSL verification to avoid config issues
    context = ssl._create_unverified_context()

    next_url = start_url
    last_page = False

    attempts = 0
    while not last_page and len(id_result) < upper_limit:
        last_page, next_url, payload = try_get_payload(next_url, context)
        if payload is None:
            continue
        for i, item in enumerate(payload["results"]):
            id_result.append(item["metadata"]["accession"])
            if len(id_result) % 1000 == 0:
                sys.stdout.write(f'{len(id_result)}-th read: {id_result[-1]}\n')
        # Don't overload the server, give it time before asking for more
        if next_url:
            time.sleep(1)
    return id_result


# In[7]:


pfam_fname = 'pfam_ids.pkl'
if os.path.exists(pfam_fname):
    with open(pfam_fname, 'rb') as f:
        id_result = pickle.load(f)
else:
    id_result = get_family_full_list(PFAM_LIST_URL)
    with open(pfam_fname, 'wb') as f:
        pickle.dump(id_result, f)


# In[8]:


print(len(id_result))


# In[9]:


def get_entry_ids(pfam_id, counter, fn_fmt):   
    start_url = PFAM_INDI_URL_PREFIX + pfam_id

    #disable SSL verification to avoid config issues
    context = ssl._create_unverified_context()

    next_url = start_url
    last_page = False

    entry_set = set()
    
    local_counter = 0
    
    FLUSH_INTERVAL = 1000
    while not last_page and next_url is not None:
        last_page, next_url, payload = try_get_payload(next_url, context)
        if payload is None:
            continue
        for i, item in enumerate(payload["results"]):
            curr_protein = item["metadata"]["accession"]
            if curr_protein not in entry_set:
                entry_set.add(curr_protein)
                local_counter += 1
                if local_counter % FLUSH_INTERVAL == 0:
                    curr_fn = fn_fmt.format(PROTEIN_LIST_FN, pfam_id, int(local_counter/FLUSH_INTERVAL))
                    # flush current to file
                    with open(curr_fn, 'wb') as f:
                        pickle.dump(entry_set, f)
                    entry_set.clear()
                    gc.collect()
                    counter.value += FLUSH_INTERVAL
                    sys.stdout.write(f'Process {os.getpid()} {local_counter}-th read in PFAM {pfam_id}\n')
                    sys.stdout.write(f'{counter.value} proteins read\n')
                    sys.stdout.flush()
                    # only check if upper limit now to avoid too frequent querying
                    if counter.value >= PFAM_UPPER_LIMIT:
                        return
        # Don't overload the server, give it time before asking for more
        time.sleep(1)
    if local_counter % FLUSH_INTERVAL > 0:
        curr_fn = fn_fmt.format(PROTEIN_LIST_FN, pfam_id, 
                                math.ceil(local_counter/FLUSH_INTERVAL+1))
        # flush current to file
        with open(curr_fn, 'wb') as f:
            pickle.dump(entry_set, f)
        entry_set.clear()
        gc.collect()
        counter.value += local_counter % FLUSH_INTERVAL
        sys.stdout.write(f'Process {os.getpid()} {local_counter}-th read in PFAM {pfam_id} in {curr_fn}\n')
        sys.stdout.write(f'{counter.value} proteins read\n')
        sys.stdout.flush()

    return 


# In[10]:


def get_protein_ids_task(pidx, counter, interval, id_result):
    start = pidx * interval
    end = min((pidx+1)*interval, len(id_result))
    curr = start
    while curr < end and counter.value < PFAM_UPPER_LIMIT:
        pfam_id = id_result[curr]
        fn_fmt = '{}_{}_{}.pkl'
        if not os.path.exists(fn_fmt.format(PROTEIN_LIST_FN, pfam_id, '1')):
            print(f"writing file to {fn_fmt.format(PROTEIN_LIST_FN, pfam_id, '1')}")
            get_entry_ids(pfam_id, counter, fn_fmt)


# In[11]:


def read_local_protein_ids():
    protein_list = []
    directory_path = 'pfam_temp'
    if os.path.isdir(directory_path):
        for filename in os.listdir(directory_path):
            curr_file = os.path.join(directory_path, filename)
            if os.path.isfile(curr_file) and 'protein_id_list' in curr_file:
                with open(curr_file, 'rb') as f:
                    protein_set = pickle.load(f)
                    protein_list += list(protein_set)
    else:
        os.makedirs(directory_path)
    return protein_list


# In[12]:


protein_list = read_local_protein_ids()
# if not reached 100k, read more
print(f'Intially, {len(protein_list)} proteins read')

if len(protein_list) < PFAM_UPPER_LIMIT:
    manager = mp.Manager()
    counter = manager.Value('i', len(protein_list))

    num_processes = NUM_PROCESSES
    interval = math.ceil(len(id_result)/num_processes)

    processes = [
        mp.Process(target=get_protein_ids_task, 
                args=(i, counter, interval, id_result)) 
        for i in range(num_processes)]
    for p in processes:
        p.start()
    for p in processes:
        p.join()
    mp.active_children()
# read again to get full list of protein ids        
protein_list = read_local_protein_ids()


# In[13]:


print(len(protein_list))


# In[14]:


def get_sequence(url_addr):   
    #disable SSL verification to avoid config issues
    context = ssl._create_unverified_context()
    _, _, payload = try_get_payload(url_addr, context)
    if payload is None:
        return None
    curr_seq = payload["metadata"]["sequence"]
    time.sleep(1)
    return curr_seq


# In[32]:


def get_sequence_task(sidx, interval, protein_list):
    start = sidx * interval
    end = min((sidx+1)*interval, len(protein_list))
    curr = start
    local_seq_counter = 0
    while curr < end:
        fidx = math.ceil(local_seq_counter/1000)
        curr_fn = f'{SEQ_FN}_{sidx}_{fidx}.txt'
        with open(os.path.join(DATA_DIR, curr_fn), 'a+') as f:
            while local_seq_counter - fidx * 1000 < 1000 and curr < end:
                protein_id = protein_list[curr]
                start_url = PROTEIN_URL_PREFIX + protein_id
                seq = get_sequence(start_url)
                if seq is not None:
                    f.write(seq + '\n')
                    local_seq_counter += 1
                curr += 1
        sys.stdout.write(f'Process {sidx} read {local_seq_counter}-th protein\n')


# In[ ]:


seq_dirname = 'sequences'
if not os.path.isdir(os.path.join(DATA_DIR, seq_dirname)):
    os.makedirs(os.path.join(DATA_DIR, seq_dirname))
    num_processes = NUM_PROCESSES
    interval = math.ceil(len(protein_list)/num_processes)
    mp.active_children()
    processes = [
        mp.Process(target=get_sequence_task, 
                args=(i, interval, protein_list)) 
        for i in range(num_processes)]
    for p in processes:
        p.start()
    for p in processes:
        p.join()
    mp.active_children()


# In[ ]:


def get_pattern(url_addr):
    #disable SSL verification to avoid config issues
    context = ssl._create_unverified_context()
    _, _, payload = try_get_payload(url_addr, context, data_type='txt')
    if payload is None:
        return None
    time.sleep(1)

    # get raw pattern
    curr_pattern = ''
    start_patt = 'PA   '
    lines = txt_text.split('\n')
    for line in lines:
        if line.startswith(start_patt):
            curr_pattern += line[len(start_patt):]
    if len(curr_pattern) == 0:
        # contains no pattern; only matrix
        return None
    return curr_pattern[:-1]


# In[ ]:


# Now let us get queries
prosite_fname = 'prosite_ids.pkl'
if os.path.exists(prosite_fname):
    with open(prosite_fname, 'rb') as f:
        prosite_id_result = pickle.load(f)
else:
    prosite_id_result = get_family_full_list(PROSITE_LIST_URL, upper_limit=1000)
    with open(prosite_fname, 'wb') as f:
        pickle.dump(prosite_id_result, f)


# In[ ]:


raw_prosite_fname = 'raw_prosite.pkl'
if os.path.exists(raw_prosite_fname):
    with open(raw_prosite_fname, 'rb') as f:
        raw_prosite = pickle.load(f)
else:
    raw_prosite = []
    for prosite_id in prosite_id_result:
        start_url = PROSITE_URL_FORMAT.format(prosite_id)
        pat = get_pattern(start_url)
        raw_prosite.append(pat)
    with open(raw_prosite_fname, 'wb') as f:
        pickle.dump(raw_prosite, f)


# In[ ]:


def prosite_to_regex(raw_prosite):
    reg = raw_prosite.replace('-', '')
    reg = reg.replace('x', '.')
    reg = reg.replace('{', '[^')
    reg = reg.replace('}', ']')
    reg = reg.replace('(', '{')
    reg = reg.replace(')', '}')
    reg = reg.replace('<', '^')
    reg = reg.replace('>', '$')
    # special case
    if reg.endswith('$]'):
        reg = reg[:-2] + ']|\z'
    return reg


# In[ ]:


query_fn = os.path.join(DATA_DIR, 'prosites.txt')
if not os.path.exists(query_fn):
    with open(query_fn, 'w') as f:
        for pat in raw_prosite:
            pat_reg = prosite_to_regex(pat)
            f.write(pat_reg + '\n')


# In[ ]:




