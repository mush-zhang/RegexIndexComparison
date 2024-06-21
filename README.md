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

- sudo apt install python3-pip

- pip3 install warcio

### Data

#### Webpage

<!-- Use python packages `warcio` to read the packets
`pip install warcio google-re2 wget`  -->

```
pushd data/webpages
./download_warc.sh
python3 extract_html.py
popd
```

#### DBLP

```
wget https://lfs.aminer.cn/lab-datasets/citation/DBLP-citation-network-Oct-19.tar.gz

```