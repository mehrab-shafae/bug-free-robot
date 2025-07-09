#ifndef ASPIRE_CORE_LOGGING_LOGGER_HPP
#define ASPIRE_CORE_LOGGING_LOGGER_HPP

#include <string>
#include <mutex>
#include <fstream>
#include <memory>

namespace aspire {
namespace core {
namespace logging {

/**
 * @brief Log levels for structured logging.
 */
enum class LogLevel {
    DEBUG = 0,
    INFO = 1,
    WARNING = 2,
    ERROR = 3,
    CRITICAL = 4
};

/**
 * @brief Thread-safe singleton logger for Aspire HTTP/2 server.
 * 
 * This class provides structured logging with different log levels,
 * file rotation, and thread-safe operations.
 */
class Logger {
public:
    /**
     * @brief Get the singleton instance of Logger.
     * @return Reference to the Logger instance.
     */
    static Logger& instance();

    /**
     * @brief Initialize logger with configuration.
     * @param log_file Path to log file.
     * @param level Minimum log level to output.
     * @param max_file_size Maximum log file size in bytes.
     * @return true if successful, false otherwise.
     */
    bool initialize(const std::string& log_file, 
                   LogLevel level = LogLevel::INFO,
                   size_t max_file_size = 10 * 1024 * 1024); // 10MB

    /**
     * @brief Log debug message.
     * @param message Log message.
     * @return true if successful, false otherwise.
     */
    bool debug(const std::string& message);

    /**
     * @brief Log info message.
     * @param message Log message.
     * @return true if successful, false otherwise.
     */
    bool info(const std::string& message);

    /**
     * @brief Log warning message.
     * @param message Log message.
     * @return true if successful, false otherwise.
     */
    bool warning(const std::string& message);

    /**
     * @brief Log error message.
     * @param message Log message.
     * @return true if successful, false otherwise.
     */
    bool error(const std::string& message);

    /**
     * @brief Log critical message.
     * @param message Log message.
     * @return true if successful, false otherwise.
     */
    bool critical(const std::string& message);

    /**
     * @brief Log access message (for HTTP requests).
     * @param message Access log message.
     * @return true if successful, false otherwise.
     */
    bool access_log(const std::string& message);

    /**
     * @brief Set minimum log level.
     * @param level New minimum log level.
     */
    void set_level(LogLevel level);

    /**
     * @brief Get current log level.
     * @return Current log level.
     */
    LogLevel get_level() const;

    /**
     * @brief Flush log buffer to file.
     */
    void flush();

private:
    Logger() = default;
    ~Logger();
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    /**
     * @brief Internal log method.
     * @param level Log level.
     * @param message Log message.
     * @return true if successful, false otherwise.
     */
    bool log(LogLevel level, const std::string& message);

    /**
     * @brief Rotate log file if necessary.
     */
    void rotate_log_file();

    /**
     * @brief Get current timestamp as string.
     * @return Timestamp string.
     */
    std::string get_timestamp() const;

    /**
     * @brief Get log level string.
     * @param level Log level.
     * @return Log level string.
     */
    std::string get_level_string(LogLevel level) const;

    mutable std::mutex mtx_;
    mutable std::mutex access_log_mtx_;
    std::unique_ptr<std::ofstream> log_file_;
    std::unique_ptr<std::ofstream> access_log_file_;
    std::string log_file_path_;
    std::string access_log_file_path_;
    LogLevel current_level_;
    size_t max_file_size_;
    size_t current_file_size_;
};

} // namespace logging
} // namespace core
} // namespace aspire

#endif // ASPIRE_CORE_LOGGING_LOGGER_HPP 