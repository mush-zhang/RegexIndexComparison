CC=gcc
CXX=g++
RM=rm -rf
CPPFLAGS=-O3 -std=c++20 -O -march=native -mfma -mavx -fomit-frame-pointer -ffp-contract=fast -fPIC -flto=auto -Wno-format -Wno-unused-result
LDFLAGS=-pthread
GUROBI_FLAGS=-I${GUROBI_HOME}/include -L${GUROBI_HOME}/lib -lgurobi_c++ -lgurobi110
RE2_FLAGS=-L/usr/local/lib -lre2

FREE_BASE_DIR=FREE
FREE_IDX_DIR=$(FREE_BASE_DIR)/Index
FREE_DIRS=$(FREE_BASE_DIR) $(FREE_IDX_DIR)
FREE_IDX=simple_query_matcher.o inverted_index.o utils/hash_pair.o $\
		 $(FREE_IDX_DIR)/free_multigram.o $\
		 $(FREE_IDX_DIR)/free_presuf.o $\
		 $(FREE_IDX_DIR)/free_multi_parallel.o

# FREE_ORI_BASE_DIR=FREE_ORIGINAL
# FREE_ORI_IDX_DIR=$(FREE_ORI_BASE_DIR)/Index
# FREE_ORI_MCH_DIR=$(FREE_ORI_BASE_DIR)/Matcher
# FREE_ORI_DIRS=$(FREE_ORI_BASE_DIR) $(FREE_ORI_IDX_DIR) $(FREE_ORI_MCH_DIR)
# FREE_ORI_MCH=$(FREE_ORI_MCH_DIR)/free_parser.o
# FREE_ORI_IDX=inverted_index.o utils/hash_pair.o $\
# 		 $(FREE_ORI_IDX_DIR)/free_multigram.o $\
# 		 $(FREE_ORI_IDX_DIR)/free_presuf.o $\
# 		 $(FREE_ORI_IDX_DIR)/free_multi_parallel.o

BEST_BASE_DIR=BEST
BEST_IDX_DIR=$(BEST_BASE_DIR)/Index
BEST_DIRS=$(BEST_BASE_DIR) $(BEST_IDX_DIR)
BEST_IDX=simple_query_matcher.o inverted_index.o $\
		 utils/rax/rax.o utils/rax/rc4rand.o $\
		 $(BEST_IDX_DIR)/best_single.o $(BEST_IDX_DIR)/best_parallel.o

# BEST_BTREE_BASE_DIR=BEST
# BEST_BTREE_IDX_DIR=$(BEST_BTREE_BASE_DIR)/Index
# BEST_BTREE_DIRS=$(BEST_BTREE_BASE_DIR) $(BEST_BTREE_IDX_DIR)
# BEST_BTREE_IDX=simple_query_matcher.o btree_index.o $\
# 		 utils/rax/rax.o utils/rax/rc4rand.o $\
# 		 $(BEST_BTREE_IDX_DIR)/best_single.o $(BEST_BTREE_IDX_DIR)/best_parallel.o

LPMS_BASE_DIR=LPMS
LPMS_IDX_DIR=$(LPMS_BASE_DIR)/Index
LPMS_DIRS=$(LPMS_BASE_DIR) $(LPMS_IDX_DIR)
LPMS_IDX=simple_query_matcher.o inverted_index.o $(LPMS_IDX_DIR)/lpms.o

# VGGRAPH_OPT_BASE_DIR=VGGRAPH_OPT
# VGGRAPH_OPT_IDX_DIR=$(VGGRAPH_OPT_BASE_DIR)/Index
# VGGRAPH_OPT_DIRS=$(VGGRAPH_OPT_BASE_DIR) $(VGGRAPH_OPT_IDX_DIR)
# VGGRAPH_OPT_IDX=simple_query_matcher.o inverted_index.o $(VGGRAPH_OPT_IDX_DIR)/vggraph_opt_index.o

VGGRAPH_GREEDY_BASE_DIR=VGGRAPH_GREEDY
VGGRAPH_GREEDY_IDX_DIR=$(VGGRAPH_GREEDY_BASE_DIR)/Index
VGGRAPH_GREEDY_DIRS=$(VGGRAPH_GREEDY_BASE_DIR) $(VGGRAPH_GREEDY_IDX_DIR)
VGGRAPH_GREEDY_IDX=simple_query_matcher.o inverted_index.o $(VGGRAPH_GREEDY_IDX_DIR)/vggraph_greedy_index.o

TRIGRAM_IDX_DIR=Trigram/Index

DIRS=. utils $(FREE_DIRS) $(FREE_ORI_DIRS) $(BEST_DIRS) $(BEST_BTREE_DIRS) $(LPMS_DIRS) $(VGGRAPH_GREEDY_DIRS) $(TRIGRAM_IDX_DIR)

GARBAGE_PATTERNS=*.o *.out *.hpp.gch
GARBAGE := $(foreach DIR,$(DIRS),$(addprefix $(DIR)/,$(GARBAGE_PATTERNS)))

.PHONY: all
all: CPPFLAGS+=-DARMA_NO_DEBUG -DNDEBUG -w
all: $(BEST_BASE_DIR)/test_best.out $\
	 $(FREE_BASE_DIR)/test_free.out $\
	 $(LPMS_BASE_DIR)/test_lpms.out $\
	 $(VGGRAPH_GREEDY_BASE_DIR)/test_vggraph_greedy.out $\
	 $(TRIGRAM_IDX_DIR)/trigram_inverted_index.o

.PHONY: debug
debug: CPPFLAGS+= -g
debug:  $(BEST_BASE_DIR)/test_best.out $\
		$(FREE_BASE_DIR)/test_free.out $\
		$(LPMS_BASE_DIR)/test_lpms.out $\
		$(VGGRAPH_GREEDY_BASE_DIR)/test_vggraph_greedy.out $\
		$(TRIGRAM_IDX_DIR)/trigram_inverted_index.o

$(TRIGRAM_IDX_DIR)/trigram_inverted_index.o: $(TRIGRAM_IDX_DIR)/trigram_inverted_index.cpp
	$(CXX) -c $(CPPFLAGS)  $^ $(RE2_FLAGS) $(LDFLAGS) -o $@

$(LPMS_BASE_DIR)/test_lpms.out: $(LPMS_IDX) $(LPMS_BASE_DIR)/test_main.cpp
	$(CXX) $(CPPFLAGS) $^ $(GUROBI_FLAGS) $(RE2_FLAGS) $(LDFLAGS) -o  $@

$(LPMS_IDX_DIR)/lpms.o: $(LPMS_IDX_DIR)/lpms.cpp
	$(CXX) -c $(CPPFLAGS) $^ $(GUROBI_FLAGS) $(LDFLAGS) -o  $@

$(VGGRAPH_GREEDY_BASE_DIR)/test_vggraph_greedy.out: $(VGGRAPH_GREEDY_IDX) $(VGGRAPH_GREEDY_BASE_DIR)/test_main.cpp
	$(CXX) $(CPPFLAGS) $^ $(RE2_FLAGS) $(LDFLAGS) -o  $@

$(VGGRAPH_GREEDY_IDX_DIR)/vggraph_greedy_index.o: $(VGGRAPH_GREEDY_IDX_DIR)/vggraph_greedy_index.cpp
	$(CXX) -c $(CPPFLAGS) $^ $(LDFLAGS) -o  $@

$(BEST_BASE_DIR)/test_best.out: $(BEST_IDX) $(BEST_BASE_DIR)/test_main.cpp
	$(CXX) $(CPPFLAGS) $^ $(RE2_FLAGS) $(LDFLAGS) -o  $@

$(BEST_IDX_DIR)/best_parallel.o: $(BEST_IDX_DIR)/parallelizable.cpp
	$(CXX) -c $(CPPFLAGS) $^ $(LDFLAGS) -o  $@

$(BEST_IDX_DIR)/best_single.o: $(BEST_IDX_DIR)/single_threaded.cpp
	$(CXX) -c $(CPPFLAGS) $^ $(LDFLAGS) -o  $@

# $(BEST_BTREE_BASE_DIR)/test_best.out: $(BEST_BTREE_IDX) $(BEST_BTREE_BASE_DIR)/test_main.cpp
# 	$(CXX) $(CPPFLAGS) $^ $(RE2_FLAGS) $(LDFLAGS) -o  $@

# $(BEST_BTREE_IDX_DIR)/best_parallel.o: $(BEST_BTREE_IDX_DIR)/parallelizable.cpp
# 	$(CXX) -c $(CPPFLAGS) $^ $(LDFLAGS) -o  $@

# $(BEST_BTREE_IDX_DIR)/best_single.o: $(BEST_BTREE_IDX_DIR)/single_threaded.cpp
# 	$(CXX) -c $(CPPFLAGS) $^ $(LDFLAGS) -o  $@

$(FREE_BASE_DIR)/test_free.out: $(FREE_IDX) $(FREE_BASE_DIR)/test_main.cpp
	$(CXX) $(CPPFLAGS) $^ $(RE2_FLAGS) $(LDFLAGS) -o  $@

$(FREE_IDX_DIR)/free_multi_parallel.o: $(FREE_IDX_DIR)/parallel_multigram_index.cpp 
	$(CXX) -c $(CPPFLAGS) $^ $(LDFLAGS) -o  $@

$(FREE_IDX_DIR)/free_presuf.o: $(FREE_IDX_DIR)/presuf_shell.cpp 
	$(CXX) -c $(CPPFLAGS) $^ $(LDFLAGS) -o  $@
 
$(FREE_IDX_DIR)/free_multigram.o: $(FREE_IDX_DIR)/multigram_index.cpp 
	$(CXX) -c $(CPPFLAGS) $^ $(LDFLAGS) -o  $@

$(VGGRAPH_OPT_BASE_DIR)/test_vggraph_opt.out: $(VGGRAPH_OPT_IDX) $(VGGRAPH_OPT_BASE_DIR)/test_main.cpp
	$(CXX) $(CPPFLAGS) $^ $(RE2_FLAGS) $(LDFLAGS) -o  $@

$(VGGRAPH_OPT_IDX_DIR)/vggraph_opt_index.o: $(VGGRAPH_OPT_IDX_DIR)/vggraph_opt_index.cpp
	$(CXX) -c $(CPPFLAGS) $^ $(LDFLAGS) -o  $@

# $(FREE_ORI_BASE_DIR)/test_free.out: $(FREE_ORI_IDX) $(FREE_ORI_MCH) $(FREE_ORI_BASE_DIR)/test_main.cpp
# 	$(CXX) $(CPPFLAGS) $^ $(RE2_FLAGS) $(LDFLAGS) -o  $@

# $(FREE_ORI_MCH_DIR)/free_parser.o: $(FREE_ORI_MCH_DIR)/query_parser.cpp
# 	$(CXX) -c $(CPPFLAGS) $^ $(LDFLAGS) -o  $@

# $(FREE_ORI_IDX_DIR)/free_multi_parallel.o: $(FREE_ORI_IDX_DIR)/parallel_multigram_index.cpp 
# 	$(CXX) -c $(CPPFLAGS) $^ $(LDFLAGS) -o  $@

# $(FREE_ORI_IDX_DIR)/free_presuf.o: $(FREE_ORI_IDX_DIR)/presuf_shell.cpp 
# 	$(CXX) -c $(CPPFLAGS) $^ $(LDFLAGS) -o  $@
 
# $(FREE_ORI_IDX_DIR)/free_multigram.o: $(FREE_ORI_IDX_DIR)/multigram_index.cpp 
# 	$(CXX) -c $(CPPFLAGS) $^ $(LDFLAGS) -o  $@

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
