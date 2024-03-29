CC=gcc
CXX=g++
RM=rm -rf
CPPFLAGS=-O3 -std=c++20 -Ofast -march=native -mfma -mavx -fomit-frame-pointer -ffp-contract=fast -flto -DARMA_NO_DEBUG
LDFLAGS=-g $(shell root-config --ldflags)

FREE_BASE_DIR=FREE
FREE_IDX_DIR=$(FREE_BASE_DIR)/Index
FREE_MCH_DIR=$(FREE_BASE_DIR)/Matcher

FREE_DIRS=$(FREE_BASE_DIR) $(FREE_IDX_DIR) $(FREE_MCH_DIR)

FREE_MCH=$(FREE_MCH_DIR)/free_parser.o
FREE_IDX=inverted.o utils/hash_pair.o $(FREE_IDX_DIR)/free_multigram.o $(FREE_IDX_DIR)/free_presuf.o 

BEST_BASE_DIR=BEST
BEST_IDX_DIR=$(BEST_BASE_DIR)/Index
BEST_MCH_DIR=$(BEST_BASE_DIR)/Matcher

BEST_DIRS=$(BEST_BASE_DIR) $(BEST_IDX_DIR) $(BEST_MCH_DIR)

BEST_IDX=inverted.o utils/rax/rax.o utils/rax/rc4rand.o $(BEST_IDX_DIR)/best_single.o

DIRS=. utils $(FREE_DIRS) $(BEST_DIRS)

GARBAGE_PATTERNS=*.o *.out *.hpp.gch
GARBAGE := $(foreach DIR,$(DIRS),$(addprefix $(DIR)/,$(GARBAGE_PATTERNS)))

.PHONY: all
all: $(BEST_BASE_DIR)/test_best.out $(FREE_BASE_DIR)/test_free.out

$(BEST_BASE_DIR)/test_best.out: $(BEST_IDX) $(BEST_BASE_DIR)/test_main.cpp
	$(CXX) $(CPPFLAGS) $^ -o $@

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

inverted.o: ngram_inverted_index.cpp
	$(CXX) -c $(CPPFLAGS) $^ -o $@

.PHONY: clean
clean:
	$(RM) $(GARBAGE)