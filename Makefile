# Aspire HTTP/2 Server Makefile
# Build system for high-performance HTTP/2 server

# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -O2 -g
CXXFLAGS += -D_GNU_SOURCE -D_REENTRANT
CXXFLAGS += -march=native -mtune=generic

# Debug flags for detailed warnings
DEBUG_FLAGS = -Wextra -Wpedantic -Wshadow -Wconversion -Wunused-parameter
DEBUG_FLAGS += -Wformat=2 -Wformat-security -Wstack-protector
DEBUG_FLAGS += -Wtrampolines -Wlogical-op -Wvector-operation-performance
DEBUG_FLAGS += -Wduplicated-cond -Wduplicated-branches -Wnull-dereference
DEBUG_FLAGS += -Wdouble-promotion -Wformat-overflow=2 -Wformat-truncation=2

# Release optimization flags
RELEASE_FLAGS = -O3 -DNDEBUG -flto -fwhole-program
RELEASE_FLAGS += -ffast-math -funroll-loops -fomit-frame-pointer
RELEASE_FLAGS += -march=native -mtune=generic -mfpmath=sse
RELEASE_FLAGS += -fstack-protector-strong -fPIC -fPIE

# Static linking flags
STATIC_FLAGS = -static-libgcc -static-libstdc++

# Directories
SRC_DIR = aspire
BUILD_DIR = build
DIST_DIR = dist
TEST_DIR = aspire/tests
LIB_DIR = lib

# Source files
CORE_SOURCES = $(filter-out %_test.cpp, $(wildcard $(SRC_DIR)/core/*/*.cpp))
MODULE_SOURCES = $(wildcard $(SRC_DIR)/modules/*/*.cpp)
MAIN_SOURCE = $(SRC_DIR)/main.cpp

# Object files
CORE_OBJECTS = $(CORE_SOURCES:%.cpp=$(BUILD_DIR)/%.o)
MODULE_OBJECTS = $(MODULE_SOURCES:%.cpp=$(BUILD_DIR)/%.o)
MAIN_OBJECT = $(BUILD_DIR)/$(MAIN_SOURCE:%.cpp=%.o)

# Test sources and objects
TEST_SOURCES = $(wildcard $(TEST_DIR)/*/*.cpp)
TEST_OBJECTS = $(TEST_SOURCES:%.cpp=$(BUILD_DIR)/%.o)

# Libraries
LIBS = -lpthread

# Target executables
TARGET = $(BUILD_DIR)/aspire_server
DEBUG_TARGET = $(BUILD_DIR)/aspire_server_debug
RELEASE_TARGET = $(DIST_DIR)/aspire_server_release
TEST_TARGET = $(BUILD_DIR)/aspire_tests

# Default target
all: $(TARGET)

# Main target (optimized)
$(TARGET): $(CORE_OBJECTS) $(MODULE_OBJECTS) $(MAIN_OBJECT)
	@echo "Linking $@..."
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LIBS)
	@echo "Build complete: $@"

# Debug target with detailed warnings
debug: CXXFLAGS += $(DEBUG_FLAGS) -g3 -O0 -DDEBUG
debug: $(DEBUG_TARGET)

$(DEBUG_TARGET): $(CORE_OBJECTS) $(MODULE_OBJECTS) $(MAIN_OBJECT)
	@echo "Linking debug version..."
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LIBS)
	@echo "Debug build complete: $@"

# Release target with maximum optimization
release: CXXFLAGS += $(RELEASE_FLAGS) $(STATIC_FLAGS)
release: $(RELEASE_TARGET)

$(RELEASE_TARGET): $(CORE_OBJECTS) $(MODULE_OBJECTS) $(MAIN_OBJECT)
	@echo "Linking release version..."
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LIBS)
	@echo "Release build complete: $@"
	@echo "Binary size:"
	@ls -lh $@

# Static release target (fully static)
static: CXXFLAGS += $(RELEASE_FLAGS) -static
static: $(DIST_DIR)/aspire_server_static

$(DIST_DIR)/aspire_server_static: $(CORE_OBJECTS) $(MODULE_OBJECTS) $(MAIN_OBJECT)
	@echo "Linking static version..."
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LIBS)
	@echo "Static build complete: $@"
	@echo "Binary size:"
	@ls -lh $@

# Core object files
$(BUILD_DIR)/$(SRC_DIR)/core/%.o: $(SRC_DIR)/core/%.cpp
	@echo "Compiling $<..."
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Module object files
$(BUILD_DIR)/$(SRC_DIR)/modules/%.o: $(SRC_DIR)/modules/%.cpp
	@echo "Compiling $<..."
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Main object file
$(MAIN_OBJECT): $(MAIN_SOURCE)
	@echo "Compiling $<..."
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Test target
test: $(TEST_TARGET)
	@echo "Running tests..."
	./$(TEST_TARGET)

$(TEST_TARGET): $(CORE_OBJECTS) $(TEST_OBJECTS)
	@echo "Linking test executable..."
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LIBS)

# Test object files
$(BUILD_DIR)/$(TEST_DIR)/%.o: $(TEST_DIR)/%.cpp
	@echo "Compiling test $<..."
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean target
clean:
	@echo "Cleaning build artifacts..."
	rm -rf $(BUILD_DIR)
	rm -rf $(DIST_DIR)
	rm -f aspire_server*

# Dependencies
-include $(CORE_OBJECTS:.o=.d)
-include $(MODULE_OBJECTS:.o=.d)
-include $(MAIN_OBJECT:.o=.d)

# Generate dependency files
$(BUILD_DIR)/%.d: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -MM -MT $(@:.d=.o) $< > $@

# Help target
help:
	@echo "Aspire HTTP/2 Server Build System"
	@echo "=================================="
	@echo "Available targets:"
	@echo "  all        - Build optimized server (default)"
	@echo "  debug      - Build with debug symbols and warnings"
	@echo "  release    - Build optimized release version"
	@echo "  static     - Build fully static binary"
	@echo "  test       - Build and run tests"
	@echo "  clean      - Remove build artifacts"
	@echo "  help       - Show this help"

# Phony targets
.PHONY: all debug release static test clean help 