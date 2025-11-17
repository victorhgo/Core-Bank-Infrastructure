# --- Makefile ---
# by Victor Correa

# Compiler
CXX := g++
CXXFLAGS := -std=c++17 -Iinclude -Wall -Wextra

# Required for libpqxx during linking process for the final program
LDFLAGS := -lpqxx -lpq

# Flags required for GoogleTest frame work
TEST_LIBS := -lgtest -lgtest_main -pthread

# Directories
SRC_DIR := src
BLD_DIR := build
OBJ_DIR := $(BLD_DIR)/objects
BIN_DIR := $(BLD_DIR)/bin
TEST_DIR := tests

CORE_SRC := $(SRC_DIR)/db_connection.cpp $(SRC_DIR)/account_service.cpp $(SRC_DIR)/transactions.cpp
CORE_OBJ := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(CORE_SRC))

# Target executable
TARGET := $(BIN_DIR)/account_service_demo

# Source and object files
SRC := $(wildcard $(SRC_DIR)/*.cpp)
OBJ := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRC))

# Test sources/objects/binary
TEST_SRC := $(wildcard $(TEST_DIR)/*.cpp)
TEST_OBJ := $(patsubst $(TEST_DIR)/%.cpp,$(OBJ_DIR)/%.test.o,$(TEST_SRC))
TEST_BIN := $(BIN_DIR)/dal_tests

# Default target
all: $(TARGET)

# Compile .cpp to .o
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Link object files into final binary
$(TARGET): $(OBJ) | $(BIN_DIR)
	$(CXX) -o $@ $^ $(LDFLAGS)

# Build tests using GoogleTest
$(OBJ_DIR)/%.test.o: $(TEST_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Test linking – ONLY link core code + test code + gtest
$(TEST_BIN): $(TEST_OBJ) $(CORE_OBJ) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS) $(TEST_LIBS)

test: $(TEST_BIN)
	./$(TEST_BIN)

# Directory creation rules
$(OBJ_DIR):
	mkdir -p $@

$(BIN_DIR):
	mkdir -p $@

# Clean generated files
clean:
	rm -rf $(BLD_DIR)

# Rebuild from scratch
rebuild: clean all

# Phony targets
.PHONY: all clean rebuild test
