# Aspire HTTP/2 Server Makefile
# Build system for high-performance HTTP/2 server

# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -O2 -g
CXXFLAGS += -D_GNU_SOURCE -D_REENTRANT

# Directories
SRC_DIR = aspire
BUILD_DIR = build
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

# Target executable
TARGET = aspire_server
TEST_TARGET = aspire_tests

# Default target
all: $(TARGET)

# Main target
$(TARGET): $(CORE_OBJECTS) $(MODULE_OBJECTS) $(MAIN_OBJECT)
	@echo "Linking $@..."
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LIBS)
	@echo "Build complete: $@"

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
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LIBS)

# Test object files
$(BUILD_DIR)/$(TEST_DIR)/%.o: $(TEST_DIR)/%.cpp
	@echo "Compiling test $<..."
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Debug target
debug: CXXFLAGS += -DDEBUG -g3 -O0
debug: $(TARGET)

# Release target
release: CXXFLAGS += -DNDEBUG -O3 -march=native
release: $(TARGET)

# Clean target
clean:
	@echo "Cleaning build artifacts..."
	rm -rf $(BUILD_DIR)
	rm -f $(TARGET) $(TEST_TARGET)

# Install target
install: $(TARGET)
	@echo "Installing Aspire HTTP/2 Server..."
	sudo cp $(TARGET) /usr/local/bin/
	sudo mkdir -p /etc/aspire
	sudo cp aspire.example.conf /etc/aspire/aspire.conf
	@echo "Installation complete"

# Uninstall target
uninstall:
	@echo "Uninstalling Aspire HTTP/2 Server..."
	sudo rm -f /usr/local/bin/$(TARGET)
	sudo rm -rf /etc/aspire
	@echo "Uninstallation complete"

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
	@echo "  all        - Build the server (default)"
	@echo "  debug      - Build with debug symbols"
	@echo "  release    - Build optimized release version"
	@echo "  test       - Build and run tests"
	@echo "  clean      - Remove build artifacts"
	@echo "  install    - Install to system"
	@echo "  uninstall  - Remove from system"
	@echo "  help       - Show this help"

# Phony targets
.PHONY: all debug release test clean install uninstall help 