CC=gcc
CXX=g++
RM=rm -rf
CPPFLAGS=-O3 -std=c++20 -Ofast -march=native -mfma -mavx -fomit-frame-pointer -ffp-contract=fast -flto -DARMA_NO_DEBUG -Wno-format -Wno-unused-result
LDFLAGS=-g $(shell root-config --ldflags)

FREE_BASE_DIR=FREE
FREE_IDX_DIR=$(FREE_BASE_DIR)/Index
FREE_MCH_DIR=$(FREE_BASE_DIR)/Matcher

FREE_DIRS=$(FREE_BASE_DIR) $(FREE_IDX_DIR) $(FREE_MCH_DIR)

FREE_MCH=$(FREE_MCH_DIR)/free_parser.o
FREE_IDX=inverted_index.o utils/hash_pair.o $(FREE_IDX_DIR)/free_multigram.o $(FREE_IDX_DIR)/free_presuf.o 

BEST_BASE_DIR=BEST
BEST_IDX_DIR=$(BEST_BASE_DIR)/Index
BEST_MCH_DIR=$(BEST_BASE_DIR)/Matcher

BEST_DIRS=$(BEST_BASE_DIR) $(BEST_IDX_DIR) $(BEST_MCH_DIR)

BEST_IDX=btree_index.o utils/rax/rax.o utils/rax/rc4rand.o $(BEST_IDX_DIR)/best_single.o $(BEST_IDX_DIR)/best_parallel.o $(BEST_MCH_DIR)/query_matcher.o

FAST_BASE_DIR=BEST
FAST_IDX_DIR=$(FAST_BASE_DIR)/Index
FAST_MCH_DIR=$(FAST_BASE_DIR)/Matcher

FAST_DIRS=$(FAST_BASE_DIR) $(FAST_IDX_DIR) $(FAST_MCH_DIR)

FAST_IDX=inverted_index.o $(FAST_IDX_DIR)/lpsm.o $(FAST_MCH_DIR)/query_matcher.o

DIRS=. utils $(FREE_DIRS) $(BEST_DIRS) $(FAST_DIRS)

GARBAGE_PATTERNS=*.o *.out *.hpp.gch
GARBAGE := $(foreach DIR,$(DIRS),$(addprefix $(DIR)/,$(GARBAGE_PATTERNS)))

.PHONY: all
all: $(FAST_BASE_DIR)/test_fast.out $(BEST_BASE_DIR)/test_best.out $(FREE_BASE_DIR)/test_free.out

$(FAST_BASE_DIR)/test_fast.out: $(FAST_IDX) $(FAST_BASE_DIR)/test_main.cpp
	$(CXX) $(CPPFLAGS) $^ -o $@

$(FAST_MCH_DIR)/query_matcher.o: $(FAST_MCH_DIR)/query_matcher.cpp
	$(CXX) -c $(CPPFLAGS) $^ -o $@

$(FAST_IDX_DIR)/lpsm.o: $(FAST_IDX_DIR)/lpsm.cpp
	$(CXX) -c $(CPPFLAGS) $^ -o $@

$(BEST_BASE_DIR)/test_best.out: $(BEST_IDX) $(BEST_BASE_DIR)/test_main.cpp
	$(CXX) $(CPPFLAGS) $^ -o $@

$(BEST_MCH_DIR)/query_matcher.o: $(BEST_MCH_DIR)/query_matcher.cpp
	$(CXX) -c $(CPPFLAGS) $^ -o $@

$(BEST_IDX_DIR)/best_parallel.o: $(BEST_IDX_DIR)/parallelizable.cpp
	$(CXX) -c $(CPPFLAGS) $^ -o $@

$(BEST_IDX_DIR)/best_single.o: $(BEST_IDX_DIR)/single_threaded.cpp
	$(CXX) -c $(CPPFLAGS) $^ -o $@

$(FREE_BASE_DIR)/test_free.out: $(FREE_IDX) $(FREE_MCH) $(FREE_BASE_DIR)/test_main.cpp
	$(CXX) $(CPPFLAGS) $^ -o $@

$(FREE_MCH_DIR)/free_parser.o: $(FREE_MCH_DIR)/query_parser.cpp
	$(CXX) -c $(CPPFLAGS) $^ -o $@

$(FREE_IDX_DIR)/free_presuf.o: $(FREE_IDX_DIR)/presuf_shell.cpp 
	$(CXX) -c $(CPPFLAGS) $^ -o $@
 
$(FREE_IDX_DIR)/free_multigram.o: $(FREE_IDX_DIR)/multigram_index.cpp 
	$(CXX) -c $(CPPFLAGS) $^ -o $@

utils/hash_pair.o: utils/hash_pair.cpp
	$(CXX) -c $(CPPFLAGS) $^ -o $@

inverted_index.o: ngram_inverted_index.cpp
	$(CXX) -c $(CPPFLAGS) $^ -o $@

btree_index.o: ngram_btree_index.cpp
	$(CXX) -c $(CPPFLAGS) $^ -o $@

# btree.o: utils/btree.cpp
# 	$(CXX) -c $(CPPFLAGS) $^ -o $@

.PHONY: clean
clean:
	$(RM) $(GARBAGE)