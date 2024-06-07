## Download from Common Crawl

### Use CC-MAIN-2013-48


### Manually choose desired webpages

1. Download Common Crawl dataset path information from the first drop-down menu in https://commoncrawl.org/get-started. For example, download 2013-48 dataset information (warc.paths.gz) at https://data.commoncrawl.org/crawl-data/CC-MAIN-2013-48/index.html

    `wget https://data.commoncrawl.org/crawl-data/[CRAWL_NAME]/warc.paths.gz`

    - untar the downloaded file. It should contain a `warc.path` file that listed all segements of the data.

    `gzip -d warc.paths.gz`

2. Open the `warc.paths` file where each line is a segment filename. Choose one or more segment(s). The lines should start with `crawl-data/[CRAWL_NAME]/segments/` and end with `.ec2.internal.warc.gz`.

2.1 Without AWS credential

    - use `https://data.commoncrawl.org/` as prefix. For example, to download one segment, do:

    ```
    $ wget https://data.commoncrawl.org/[SEGMENT_FILENAME]
    ```

2.2 AWS-cli (for detailed installation guide, visit https://docs.aws.amazon.com/cli/latest/userguide/getting-started-install.html)

    ```
    curl "https://awscli.amazonaws.com/awscli-exe-linux-x86_64.zip" -o "awscliv2.zip"
    unzip awscliv2.zip
    sudo ./aws/install

    - use `s3://commoncrawl/` prefix. For example, to download one segment, do:

    ```
    aws s3 cp s3://commoncrawl/[SEGMENT_FILENAME] .
    ```

3. Decompress the file

`gzip -d [filename]`

## Extract HTML to text files

`python3 extract_html.py`
