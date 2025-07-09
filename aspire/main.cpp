#include <iostream>
#include <string>
#include <csignal>
#include <memory>
#include <thread>

#include "core/config/Config.hpp"
#include "core/logging/Logger.hpp"
#include "core/utils/Timer.hpp"
#include "core/networking/Server.hpp"

using namespace aspire::core;

// Global variables for signal handling
static volatile sig_atomic_t g_shutdown_requested = 0;

/**
 * @brief Signal handler for graceful shutdown.
 * @param signal Signal number.
 */
void signal_handler(int signal) {
    if (signal == SIGINT || signal == SIGTERM) {
        g_shutdown_requested = 1;
        std::cout << "\nShutdown signal received. Gracefully stopping server...\n";
    }
}

/**
 * @brief Initialize signal handlers.
 * @return true if successful, false otherwise.
 */
bool initialize_signal_handlers() {
    struct sigaction sa;
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    
    if (sigaction(SIGINT, &sa, nullptr) == -1) {
        std::cerr << "Failed to set SIGINT handler\n";
        return false;
    }
    
    if (sigaction(SIGTERM, &sa, nullptr) == -1) {
        std::cerr << "Failed to set SIGTERM handler\n";
        return false;
    }
    
    return true;
}

/**
 * @brief Initialize configuration system.
 * @param config_path Path to configuration file.
 * @return true if successful, false otherwise.
 */
bool initialize_configuration(const std::string& config_path) {
    auto& config = config::Config::instance();
    
    if (!config.load(config_path)) {
        std::cerr << "Failed to load configuration from: " << config_path << "\n";
        return false;
    }
    
    std::cout << "Configuration loaded successfully\n";
    return true;
}

/**
 * @brief Initialize logging system.
 * @return true if successful, false otherwise.
 */
bool initialize_logging() {
    auto& logger = logging::Logger::instance();
    
    // Get log file path from config or use default
    auto& config = config::Config::instance();
    std::string log_file = config.get_string("log_file", "logs/aspire.log");
    
    if (!logger.initialize(log_file, logging::LogLevel::INFO)) {
        std::cerr << "Failed to initialize logging system\n";
        return false;
    }
    
    logger.info("Aspire HTTP/2 Server starting up");
    return true;
}

/**
 * @brief Print server banner and version information.
 */
void print_banner() {
    std::cout << "========================================\n";
    std::cout << "    Aspire HTTP/2 Server v1.0.0\n";
    std::cout << "    High-Performance HTTP/2 Server\n";
    std::cout << "========================================\n";
    std::cout << "Built with C++20 for Linux\n";
    std::cout << "========================================\n\n";
}

/**
 * @brief Main application entry point.
 * @param argc Argument count.
 * @param argv Argument vector.
 * @return Exit code.
 */
int main(int argc, char* argv[]) {
    print_banner();
    
    // Parse command line arguments
    std::string config_path = "aspire.conf";
    if (argc > 1) {
        config_path = argv[1];
    }
    
    std::cout << "Using configuration file: " << config_path << "\n";
    
    // Initialize signal handlers
    if (!initialize_signal_handlers()) {
        std::cerr << "Failed to initialize signal handlers\n";
        return 1;
    }
    
    // Initialize configuration
    if (!initialize_configuration(config_path)) {
        std::cerr << "Failed to initialize configuration\n";
        return 1;
    }
    
    // Initialize logging
    if (!initialize_logging()) {
        std::cerr << "Failed to initialize logging\n";
        return 1;
    }
    
    auto& logger = logging::Logger::instance();
    auto& config = config::Config::instance();
    
    // Log startup information
    logger.info("Server initialization completed");
    logger.info("Configuration loaded from: " + config_path);
    
    // Get server configuration
    int port = config.get_int("port", 9080);
    int backlog = config.get_int("backlog", 100);
    
    logger.info("Server configured for port: " + std::to_string(port));
    logger.info("Backlog size: " + std::to_string(backlog));
    
    // Initialize HTTP server
    auto server = std::make_unique<networking::Server>(port, backlog);
    
    if (!server->initialize()) {
        logger.error("Failed to initialize server");
        return 1;
    }
    
    // Set request handler
    server->set_request_handler([](std::shared_ptr<networking::Connection> connection) {
        // Simple HTTP response for now
        std::string response = 
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/plain\r\n"
            "Content-Length: 13\r\n"
            "\r\n"
            "Hello, World!";
        
        connection->write_data(response.c_str(), response.length());
    });
    
    logger.info("Server initialized successfully");
    logger.info("Starting server on port " + std::to_string(port));
    
    // Start server in a separate thread
    std::thread server_thread([&server]() {
        server->start();
    });
    
    // Wait for shutdown signal
    while (!g_shutdown_requested) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    // Stop server
    logger.info("Shutting down server...");
    server->stop();
    server_thread.join();
    
    logger.info("Server shutdown requested");
    logger.info("Aspire HTTP/2 Server stopped");
    
    std::cout << "Server stopped gracefully\n";
    return 0;
}
