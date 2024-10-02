# Compiler settings
CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -pedantic -O2
LDFLAGS :=

# Directories
SRC_DIR := .
OBJ_DIR := obj
BIN_DIR := bin
INC_DIR := .

# Source files
SRCS := $(wildcard $(SRC_DIR)/*.cpp)
OBJS := $(SRCS:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

# Main target
TARGET := $(BIN_DIR)/diagnostic_tool

# Phony targets
.PHONY: all clean debug release info

# Default target
all: info release

# Release build
release: CXXFLAGS += -DNDEBUG
release: $(TARGET)

# Debug build
debug: CXXFLAGS += -g -DDEBUG
debug: $(TARGET)

# Link the target executable
$(TARGET): $(OBJS) | $(BIN_DIR)
	@echo "Linking $@..."
	@$(CXX) $(CXXFLAGS) $(OBJS) -o $@ $(LDFLAGS)

# Compile source files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	@echo "Compiling $<..."
	@$(CXX) $(CXXFLAGS) -I$(INC_DIR) -MMD -c $< -o $@

# Include dependencies
-include $(DEPS)

# Create directories
$(BIN_DIR) $(OBJ_DIR):
	@mkdir -p $@

# Clean up build artifacts
clean:
	@echo "Cleaning up..."
	@rm -rf $(OBJ_DIR) $(BIN_DIR)

# Print build information
info:
	@echo "Source files:"
	@echo "$(SRCS)"
	@echo
	@echo "Object files:"
	@echo "$(OBJS)"
	@echo
	@echo "Include directory:"
	@echo "$(INC_DIR)"
	@echo
	@echo "Target:"
	@echo "$(TARGET)"
	@echo

# Help target
.PHONY: help
help:
	@echo "Available targets:"
	@echo "  all (default) - Show info and build the release version"
	@echo "  debug         - Build with debug symbols"
	@echo "  release       - Build the release version"
	@echo "  clean         - Remove build artifacts"
	@echo "  info          - Display build information"
	@echo "  help          - Display this help message"
