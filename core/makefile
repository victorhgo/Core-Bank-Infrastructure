# --- Makefile ---
# by Victor Correa

# Compiler
CXX := g++
CXXFLAGS := -std=c++17 -Iinclude -Wall -Wextra

# Required for libpqxx during linking process for the final program
LDFLAGS := -lpqxx -lpq

# Directories
SRC_DIR := src
BLD_DIR := build
OBJ_DIR := $(BLD_DIR)/objects
BIN_DIR := $(BLD_DIR)/bin

# Target executable
TARGET := $(BIN_DIR)/db_connection_demo

# Source and object files
SRC := $(wildcard $(SRC_DIR)/*.cpp)
OBJ := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRC))

# Default target
all: $(TARGET)

# Compile .cpp to .o
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Link object files into final binary
$(TARGET): $(OBJ) | $(BIN_DIR)
	$(CXX) -o $@ $^ $(LDFLAGS)

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
.PHONY: all clean rebuild
