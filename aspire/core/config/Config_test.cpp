#include "Config.hpp"
#include <iostream>
#include <cassert>
#include <fstream>

using namespace aspire::core::config;

/**
 * @brief Test configuration loading and retrieval.
 * @return true if all tests pass, false otherwise.
 */
bool test_config_loading() {
    std::cout << "Testing configuration loading...\n";
    
    // Create a temporary config file
    std::ofstream test_file("test_config.conf");
    test_file << "# Test configuration file\n";
    test_file << "port=8080\n";
    test_file << "backlog=200\n";
    test_file << "debug=true\n";
    test_file << "log_level=INFO\n";
    test_file.close();
    
    auto& config = Config::instance();
    
    // Test loading configuration
    if (!config.load("test_config.conf")) {
        std::cerr << "Failed to load test configuration\n";
        return false;
    }
    
    // Test string retrieval
    if (config.get_string("port") != "8080") {
        std::cerr << "String retrieval failed\n";
        return false;
    }
    
    // Test integer retrieval
    if (config.get_int("port") != 8080) {
        std::cerr << "Integer retrieval failed\n";
        return false;
    }
    
    if (config.get_int("backlog") != 200) {
        std::cerr << "Integer retrieval failed\n";
        return false;
    }
    
    // Test boolean retrieval
    if (!config.get_bool("debug")) {
        std::cerr << "Boolean retrieval failed\n";
        return false;
    }
    
    // Test default values
    if (config.get_string("nonexistent", "default") != "default") {
        std::cerr << "Default value test failed\n";
        return false;
    }
    
    if (config.get_int("nonexistent", 42) != 42) {
        std::cerr << "Default value test failed\n";
        return false;
    }
    
    // Test key existence
    if (!config.has_key("port")) {
        std::cerr << "Key existence test failed\n";
        return false;
    }
    
    if (config.has_key("nonexistent")) {
        std::cerr << "Key existence test failed\n";
        return false;
    }
    
    // Clean up
    std::remove("test_config.conf");
    
    std::cout << "Configuration loading tests passed\n";
    return true;
}

/**
 * @brief Test configuration setting.
 * @return true if all tests pass, false otherwise.
 */
bool test_config_setting() {
    std::cout << "Testing configuration setting...\n";
    
    auto& config = Config::instance();
    
    // Test setting values
    config.set("test_key", "test_value");
    config.set("test_int", "123");
    config.set("test_bool", "true");
    
    // Test retrieval of set values
    if (config.get_string("test_key") != "test_value") {
        std::cerr << "Set value retrieval failed\n";
        return false;
    }
    
    if (config.get_int("test_int") != 123) {
        std::cerr << "Set integer retrieval failed\n";
        return false;
    }
    
    if (!config.get_bool("test_bool")) {
        std::cerr << "Set boolean retrieval failed\n";
        return false;
    }
    
    std::cout << "Configuration setting tests passed\n";
    return true;
}

/**
 * @brief Test configuration keys retrieval.
 * @return true if all tests pass, false otherwise.
 */
bool test_config_keys() {
    std::cout << "Testing configuration keys...\n";
    
    auto& config = Config::instance();
    
    // Clear and set some test keys
    config.set("key1", "value1");
    config.set("key2", "value2");
    config.set("key3", "value3");
    
    auto keys = config.get_keys();
    
    // Check if all expected keys are present
    bool found_key1 = false, found_key2 = false, found_key3 = false;
    
    for (const auto& key : keys) {
        if (key == "key1") found_key1 = true;
        if (key == "key2") found_key2 = true;
        if (key == "key3") found_key3 = true;
    }
    
    if (!found_key1 || !found_key2 || !found_key3) {
        std::cerr << "Keys retrieval test failed\n";
        return false;
    }
    
    std::cout << "Configuration keys tests passed\n";
    return true;
}

/**
 * @brief Main test function.
 * @return Exit code.
 */
int main() {
    std::cout << "Running Config class tests...\n\n";
    
    bool all_tests_passed = true;
    
    // Run all tests
    if (!test_config_loading()) {
        all_tests_passed = false;
    }
    
    if (!test_config_setting()) {
        all_tests_passed = false;
    }
    
    if (!test_config_keys()) {
        all_tests_passed = false;
    }
    
    if (all_tests_passed) {
        std::cout << "\nAll Config tests passed!\n";
        return 0;
    } else {
        std::cout << "\nSome Config tests failed!\n";
        return 1;
    }
} 