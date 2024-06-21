#!/usr/bin/env python
# coding: utf-8

import os
import sys
from warcio.archiveiterator import ArchiveIterator

raw_data_dir = '.'
processed_data_dir = 'processed'
os.makedirs(processed_data_dir, exist_ok=True)

upper_limit = 700000

i = 0

for filename in os.listdir(raw_data_dir):
    if '.warc' not in filename:
        continue
    full_fname = os.path.join(raw_data_dir, filename)
    with open(full_fname, 'rb') as stream:
        j = 0
        for record in ArchiveIterator(stream):
            if record.rec_type == 'warcinfo':
                print('-------------------')
                print(record.raw_stream.read())
                print('\n-------------------')

            elif record.rec_type == 'response':
                header = record.http_headers.get_header('Content-Type')
                if header is not None and 'text/html' in header:
                    with open(os.path.join(processed_data_dir, f'{filename}_{j}.txt'), 'wb') as file:
                        file.write(record.content_stream().read())
                    j += 1
                    if i+j > upper_limit:
                        sys.exit()

    i += j
