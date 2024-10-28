CC=gcc
CXX=g++
RM=rm -rf
CPPFLAGS=-O3 -std=c++20 -Ofast -march=native -mfma -mavx -fomit-frame-pointer -ffp-contract=fast -fPIC -flto -Wno-format -Wno-unused-result
LDFLAGS=-pthread
GUROBI_FLAGS=-I${GUROBI_HOME}/include -L${GUROBI_HOME}/lib -lgurobi_c++ -lgurobi110
RE2_FLAGS=-L/usr/local/lib -lre2

FREE_BASE_DIR=FREE
FREE_IDX_DIR=$(FREE_BASE_DIR)/Index
FREE_MCH_DIR=$(FREE_BASE_DIR)/Matcher
FREE_DIRS=$(FREE_BASE_DIR) $(FREE_IDX_DIR) $(FREE_MCH_DIR)
FREE_MCH=$(FREE_MCH_DIR)/free_parser.o
FREE_IDX=inverted_index.o utils/hash_pair.o $\
		 $(FREE_IDX_DIR)/free_multigram.o $\
		 $(FREE_IDX_DIR)/free_presuf.o $\
		 $(FREE_IDX_DIR)/free_multi_parallel.o

BEST_BASE_DIR=BEST
BEST_IDX_DIR=$(BEST_BASE_DIR)/Index
BEST_DIRS=$(BEST_BASE_DIR) $(BEST_IDX_DIR)
BEST_IDX=simple_query_matcher.o btree_index.o $\
		 utils/rax/rax.o utils/rax/rc4rand.o $\
		 $(BEST_IDX_DIR)/best_single.o $(BEST_IDX_DIR)/best_parallel.o

FAST_BASE_DIR=FAST
FAST_IDX_DIR=$(FAST_BASE_DIR)/Index
FAST_DIRS=$(FAST_BASE_DIR) $(FAST_IDX_DIR)
FAST_IDX=simple_query_matcher.o inverted_index.o $(FAST_IDX_DIR)/lpms.o

DIRS=. utils $(FREE_DIRS) $(BEST_DIRS) $(FAST_DIRS)

GARBAGE_PATTERNS=*.o *.out *.hpp.gch
GARBAGE := $(foreach DIR,$(DIRS),$(addprefix $(DIR)/,$(GARBAGE_PATTERNS)))

.PHONY: all
all: CPPFLAGS+=-DARMA_NO_DEBUG -DNDEBUG -w
all: $(BEST_BASE_DIR)/test_best.out $\
	 $(FREE_BASE_DIR)/test_free.out $\
	 $(FAST_BASE_DIR)/test_fast.out 

.PHONY: debug
debug: CPPFLAGS+= -g
debug:  $(BEST_BASE_DIR)/test_best.out $\
		$(FREE_BASE_DIR)/test_free.out $\
		$(FAST_BASE_DIR)/test_fast.out 

$(FAST_BASE_DIR)/test_fast.out: $(FAST_IDX) $(FAST_BASE_DIR)/test_main.cpp
	$(CXX) $(CPPFLAGS) $^ $(GUROBI_FLAGS) $(RE2_FLAGS) $(LDFLAGS) -o  $@

$(FAST_IDX_DIR)/lpms.o: $(FAST_IDX_DIR)/lpms.cpp
	$(CXX) -c $(CPPFLAGS) $^ $(GUROBI_FLAGS) $(LDFLAGS) -o  $@

$(BEST_BASE_DIR)/test_best.out: $(BEST_IDX) $(BEST_BASE_DIR)/test_main.cpp
	$(CXX) $(CPPFLAGS) $^ $(RE2_FLAGS) $(LDFLAGS) -o  $@

$(BEST_IDX_DIR)/best_parallel.o: $(BEST_IDX_DIR)/parallelizable.cpp
	$(CXX) -c $(CPPFLAGS) $^ $(LDFLAGS) -o  $@

$(BEST_IDX_DIR)/best_single.o: $(BEST_IDX_DIR)/single_threaded.cpp
	$(CXX) -c $(CPPFLAGS) $^ $(LDFLAGS) -o  $@

$(FREE_BASE_DIR)/test_free.out: $(FREE_IDX) $(FREE_MCH) $(FREE_BASE_DIR)/test_main.cpp
	$(CXX) $(CPPFLAGS) $^ $(RE2_FLAGS) $(LDFLAGS) -o  $@

$(FREE_MCH_DIR)/free_parser.o: $(FREE_MCH_DIR)/query_parser.cpp
	$(CXX) -c $(CPPFLAGS) $^ $(LDFLAGS) -o  $@

$(FREE_IDX_DIR)/free_multi_parallel.o: $(FREE_IDX_DIR)/parallel_multigram_index.cpp 
	$(CXX) -c $(CPPFLAGS) $^ $(LDFLAGS) -o  $@

$(FREE_IDX_DIR)/free_presuf.o: $(FREE_IDX_DIR)/presuf_shell.cpp 
	$(CXX) -c $(CPPFLAGS) $^ $(LDFLAGS) -o  $@
 
$(FREE_IDX_DIR)/free_multigram.o: $(FREE_IDX_DIR)/multigram_index.cpp 
	$(CXX) -c $(CPPFLAGS) $^ $(LDFLAGS) -o  $@

simple_query_matcher.o: simple_query_matcher.cpp
	$(CXX) -c $(CPPFLAGS) $^ $(RE2_FLAGS) $(LDFLAGS) -o  $@

inverted_index.o: ngram_inverted_index.cpp
	$(CXX) -c $(CPPFLAGS) $^ $(LDFLAGS) -o  $@

btree_index.o: ngram_btree_index.cpp
	$(CXX) -c $(CPPFLAGS) $^ $(LDFLAGS) -o  $@

# btree.o: utils/btree.cpp
# 	$(CXX) -c $(CPPFLAGS) $^ $(LDFLAGS) -o  $@

utils/hash_pair.o: utils/hash_pair.cpp
	$(CXX) -c $(CPPFLAGS) $^ $(LDFLAGS) -o  $@

.PHONY: clean
clean:
	$(RM) $(GARBAGE)