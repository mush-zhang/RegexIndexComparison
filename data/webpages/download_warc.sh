#! /bin/bash

PATH_FILE="warc.paths"

FILE_LIMIT=1

wget https://data.commoncrawl.org/crawl-data/CC-MAIN-2013-48/${PATH_FILE}.gz
gzip -d ${PATH_FILE}.gz

num_file_read=0
while read -r line; do
    wget https://data.commoncrawl.org/${line}

    num_file_read=$((num_file_read+1))
    echo $num_file_read

    if [[ "${num_file_read}" -gt "${FILE_LIMIT}" ]]; then
        break
    fi
done < "${PATH_FILE}"