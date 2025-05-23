#! /bin/bash

PATH_FILE="warc.paths"

FILE_LIMIT=70

sudo mkdir /mnt/webtemp

wget https://data.commoncrawl.org/crawl-data/CC-MAIN-2013-48/${PATH_FILE}.gz
gzip -d ${PATH_FILE}.gz

num_file_read=0
while read -r line; do
    if [[ "${num_file_read}" -ge "${FILE_LIMIT}" ]]; then
        break
    fi
    wget https://data.commoncrawl.org/${line}
    num_file_read=$((num_file_read+1))
done < "${PATH_FILE}"
