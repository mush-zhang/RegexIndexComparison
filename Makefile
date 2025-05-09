CC=gcc
CXX=g++

CPPFLAGS=-O3 -std=c++20 -Ofast -march=native -mfma -mavx -fomit-frame-pointer -ffp-contract=fast -fPIC -flto=auto -Wno-format -Wno-unused-result
LDFLAGS=-pthread -lstdc++fs
GUROBI_FLAGS=-I${GUROBI_HOME}/include -L${GUROBI_HOME}/lib -lgurobi_c++ -lgurobi110
RE2_FLAGS=-L/usr/local/lib -lre2

SRC_DIR=src

FREE_BASE_DIR=$(SRC_DIR)/FREE
FREE_IDX_DIR=$(FREE_BASE_DIR)/Index
FREE_DIRS=$(FREE_IDX_DIR)

LPMS_BASE_DIR=$(SRC_DIR)/LPMS
LPMS_IDX_DIR=$(LPMS_BASE_DIR)/Index
LPMS_DIRS=$(LPMS_IDX_DIR)

BEST_BASE_DIR=$(SRC_DIR)/BEST
BEST_IDX_DIR=$(BEST_BASE_DIR)/Index
BEST_DIRS=$(BEST_IDX_DIR)

DIRS=. $(SRC_DIR)/utils $(FREE_DIRS) $(BEST_DIRS) $(LPMS_DIRS)
OBJECT_PATTERNS=*.o
OBJECTS := $(foreach DIR,$(DIRS),$(addprefix $(DIR)/,$(OBJECT_PATTERNS)))

OBJECT_LIST=$(shell echo $(OBJECTS))
$(info $(OBJECT_LIST))

.PHONY: debug
debug: CPPFLAGS+= -g
debug:benchmark.out

.PHONY: all
all: CPPFLAGS+=-DARMA_NO_DEBUG -DNDEBUG -w
all: benchmark.out

# benchmark.out: $(SRC_DIR)/utils/rax/rax.o $(SRC_DIR)/utils/rax/rc4rand.o $\
# 			   $(SRC_DIR)/btree_index.o $(SRC_DIR)/inverted_index.o $\
# 			   $(SRC_DIR)/simple_query_matcher.o $(SRC_DIR)/utils/hash_pair.o $\
# 			   $(FREE_IDX_DIR)/free_multigram.o $(FREE_IDX_DIR)/free_presuf.o $\
# 			   $(FREE_IDX_DIR)/free_multi_parallel.o $(FREE_MCH_DIR)/free_parser.o $\
# 			   $(BEST_IDX_DIR)/best_single.o $(BEST_IDX_DIR)/best_parallel.o $\
# 			   $(LPMS_IDX_DIR)/lpms.o $\
# 			   benchmarks/utils.o benchmarks/benchmark.cpp
# 	$(CXX) $(CPPFLAGS) $^ $(LDFLAGS) $(GUROBI_FLAGS) $(RE2_FLAGS) -o $@
benchmark.out: $(SRC_DIR)/utils/rax/rax.o $(SRC_DIR)/utils/rax/rc4rand.o $\
			   $(SRC_DIR)/inverted_index.o $\
			   $(SRC_DIR)/simple_query_matcher.o $(SRC_DIR)/utils/hash_pair.o $\
			   $(FREE_IDX_DIR)/free_multigram.o $(FREE_IDX_DIR)/free_presuf.o $\
			   $(FREE_IDX_DIR)/free_multi_parallel.o $\
			   $(BEST_IDX_DIR)/best_single.o $(BEST_IDX_DIR)/best_parallel.o $\
			   $(LPMS_IDX_DIR)/lpms.o $\
			   benchmarks/utils.o benchmarks/benchmark.cpp
	$(CXX) $(CPPFLAGS) $^ $(LDFLAGS) $(GUROBI_FLAGS) $(RE2_FLAGS) -o $@

benchmarks/utils.o: benchmarks/utils.cpp
	$(CXX) -c $(CPPFLAGS) $^ $(LDFLAGS) $(GUROBI_FLAGS) $(RE2_FLAGS) -o  $@

.PHONY: clean
clean:
	rm -f benchmark.out benchmarks/utils.o