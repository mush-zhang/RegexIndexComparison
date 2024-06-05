# RegexIndexComparison

## Pre-requisite

- git submodule update --init --recursive

- sudo apt-get -y install pkg-config

- Gurobi for Lp solver: https://www.gurobi.com/downloads/gurobi-software/

```
cd src/utils/abseil-cpp
mkdir build && cd build
cmake -DCMAKE_INSTALL_PREFIX=/usr/local  -DCMAKE_CXX_STANDARD=20 -DCMAKE_POSITION_INDEPENDENT_CODE=TRUE -DABSL_PROPAGATE_CXX_STD=TRUE ..
make -j
sudo make install
```
### Data

#### Webpage

1. Download Common Crawl 2013-48 dataset segment information (warc.paths.gz) at https://data.commoncrawl.org/crawl-data/CC-MAIN-2013-48/index.html

    `wget https://data.commoncrawl.org/crawl-data/CC-MAIN-2013-48/warc.paths.gz`

    - untar the downloaded file. It should contain a `warc.path` file that listed all segements of the data.

    `gzip -d warc.paths.gz`

2.1 Without AWS credential

    - use `https://data.commoncrawl.org/` as prefix. For example, to download one segment, do:

    ```
    $ wget https://data.commoncrawl.org/crawl-data/CC-MAIN-2013-48/segments/1386163035819/warc/CC-MAIN-20131204131715-00000-ip-10-33-133-15.ec2.internal.warc.gz
    ```


2.2 AWS-cli (for detailed installation guide, visit https://docs.aws.amazon.com/cli/latest/userguide/getting-started-install.html)

    ```
    curl "https://awscli.amazonaws.com/awscli-exe-linux-x86_64.zip" -o "awscliv2.zip"
    unzip awscliv2.zip
    sudo ./aws/install

    - use `s3://commoncrawl/` prefix. For example, to download one segment, do:

    ```
    aws s3 cp s3://commoncrawl/crawl-data/CC-MAIN-2013-48/segments/1386163035819/warc/CC-MAIN-20131204131715-00000-ip-10-33-133-15.ec2.internal.warc.gz data/webpages
    ```

3. Decompress the file

`gzip -d [filename]`

4. Use `warcio` `cdx-toolkit` to read the packets (https://pypi.org/project/cdx-toolkit/)

`pip install cdx-toolkit` 
