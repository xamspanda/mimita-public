# Makefile for mimita-public (macOS and Linux)

# Detect OS
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
    PLATFORM := linux
else ifeq ($(UNAME_S),Darwin)
    PLATFORM := macos
else
    $(error Unsupported platform: $(UNAME_S))
endif

# Compiler
CXX := clang++
CC := clang

# Executable name
TARGET := mimita

# Directories
SRC_DIR := src
BUILD_DIR := build
INCLUDE_DIRS := -Iinclude -Isrc -Iexternal/include

# GLFW flags (using pkg-config)
GLFW_FLAGS := $(shell pkg-config --cflags glfw3 2>/dev/null || echo "")
GLFW_LIBS := $(shell pkg-config --libs glfw3 2>/dev/null || echo "")

# Compiler flags
CXXFLAGS := -std=c++23 -O2 -Wall $(INCLUDE_DIRS) $(GLFW_FLAGS)
CFLAGS := $(INCLUDE_DIRS) $(GLFW_FLAGS)

# Platform-specific linker flags
ifeq ($(PLATFORM),macos)
    LDFLAGS := $(GLFW_LIBS) -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo
else ifeq ($(PLATFORM),linux)
    LDFLAGS := $(GLFW_LIBS) -lGL -lX11 -lpthread -ldl
endif

# Find all source files
CPP_SOURCES := $(shell find $(SRC_DIR) -name '*.cpp')
C_SOURCES := $(shell find $(SRC_DIR) -name '*.c')

# Generate object file names (relative to src dir)
CPP_OBJECTS := $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(CPP_SOURCES))
C_OBJECTS := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(C_SOURCES))
OBJECTS := $(CPP_OBJECTS) $(C_OBJECTS)

# Create build directories
OBJECT_DIRS := $(sort $(dir $(OBJECTS)))

# Default target
.PHONY: all
all: $(TARGET)

# Link executable
$(TARGET): $(OBJECTS)
	@echo "Linking $(TARGET)..."
	$(CXX) $(OBJECTS) -o $(TARGET) $(LDFLAGS)
	@echo "Build successful!"

# Compile C++ sources
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	@echo "Compiling $<..."
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Compile C sources
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) -c $< -o $@

# Run the executable
.PHONY: run
run: $(TARGET)
	@echo "Running $(TARGET)..."
	./$(TARGET)

# Clean build artifacts
.PHONY: clean
clean:
	@echo "Cleaning..."
	rm -rf $(BUILD_DIR) $(TARGET)
	@echo "Clean complete!"

# Print debug info
.PHONY: debug
debug:
	@echo "Sources: $(CPP_SOURCES) $(C_SOURCES)"
	@echo "Objects: $(OBJECTS)"
	@echo "Object dirs: $(OBJECT_DIRS)"
	@echo "CXXFLAGS: $(CXXFLAGS)"
	@echo "LDFLAGS: $(LDFLAGS)"

