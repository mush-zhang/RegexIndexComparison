# VGGraph_Opt Index Makefile

CXX = g++
CPPFLAGS = -O3 -std=c++17 -march=native -fomit-frame-pointer -fPIC -Wno-format -Wno-unused-result
LDFLAGS = -pthread
RE2_FLAGS = -L/usr/local/lib -lre2

# Base files needed
BASE_OBJECTS = ../ngram_inverted_index.o ../simple_query_matcher.o ../utils/utils.o

# VGGraph_Opt specific files
VGGRAPH_OPT_OBJECTS = Index/vggraph_opt_index.o
ALL_OBJECTS = $(BASE_OBJECTS) $(VGGRAPH_OPT_OBJECTS)

# Target executable
TARGET = test_vggraph_opt.out

# Build target
$(TARGET): $(ALL_OBJECTS) test_main.cpp
	$(CXX) $(CPPFLAGS) $^ $(RE2_FLAGS) $(LDFLAGS) -o $@

# Pattern rules for object files
%.o: %.cpp
	$(CXX) -c $(CPPFLAGS) $< $(LDFLAGS) -o $@

# Build base dependencies
../%.o: ../%.cpp
	$(CXX) -c $(CPPFLAGS) $< $(LDFLAGS) -o $@

# Clean
clean:
	rm -f $(ALL_OBJECTS) $(TARGET)

# Phony targets
.PHONY: clean help

# Help target
help:
	@echo "Available targets:"
	@echo "  $(TARGET)    - Build the VGGraph_Opt test executable"
	@echo "  clean        - Remove all build artifacts"
	@echo "  help         - Show this help message"
	@echo ""
	@echo "Usage example:"
	@echo "  make $(TARGET)"
	@echo "  ./$(TARGET) dataset.txt queries.txt 0.1 4 8"
