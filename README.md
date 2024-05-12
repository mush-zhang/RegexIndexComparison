# RegexIndexComparison

### Pre-requisite

- git submodule update --init --recursive

- sudo apt-get -y install pkg-config

- Gurobi for Lp solver: https://www.gurobi.com/downloads/gurobi-software/

- git submodule init

- git submodule update

```
cd src/utils/abseil-cpp
mkdir build && cd build
cmake -DCMAKE_INSTALL_PREFIX=/usr/local  -DCMAKE_CXX_STANDARD=20 -DCMAKE_POSITION_INDEPENDENT_CODE=TRUE -DABSL_PROPAGATE_CXX_STD=TRUE ..
make -j
sudo make install
```