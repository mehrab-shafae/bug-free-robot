#include "Logger.hpp"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <sys/stat.h>
#include <unistd.h>

namespace aspire {
namespace core {
namespace logging {

Logger& Logger::instance() {
    static Logger instance;
    return instance;
}

Logger::~Logger() {
    flush();
}

bool Logger::initialize(const std::string& log_file, 
                       LogLevel level,
                       size_t max_file_size) {
    std::lock_guard<std::mutex> lock(mtx_);
    
    current_level_ = level;
    max_file_size_ = max_file_size;
    current_file_size_ = 0;
    
    // Create log directory if it doesn't exist
    size_t last_slash = log_file.find_last_of('/');
    if (last_slash != std::string::npos) {
        std::string log_dir = log_file.substr(0, last_slash);
        if (mkdir(log_dir.c_str(), 0755) != 0 && errno != EEXIST) {
            return false;
        }
    }
    
    // Open main log file
    log_file_ = std::make_unique<std::ofstream>(log_file, std::ios::app);
    if (!log_file_->is_open()) {
        return false;
    }
    
    log_file_path_ = log_file;
    
    // Open access log file
    std::string access_log_path = log_file.substr(0, log_file.find_last_of('.')) + "_access.log";
    access_log_file_ = std::make_unique<std::ofstream>(access_log_path, std::ios::app);
    if (!access_log_file_->is_open()) {
        return false;
    }
    
    access_log_file_path_ = access_log_path;
    
    return true;
}

bool Logger::debug(const std::string& message) {
    return log(LogLevel::DEBUG, message);
}

bool Logger::info(const std::string& message) {
    return log(LogLevel::INFO, message);
}

bool Logger::warning(const std::string& message) {
    return log(LogLevel::WARNING, message);
}

bool Logger::error(const std::string& message) {
    return log(LogLevel::ERROR, message);
}

bool Logger::critical(const std::string& message) {
    return log(LogLevel::CRITICAL, message);
}

bool Logger::access_log(const std::string& message) {
    std::lock_guard<std::mutex> lock(access_log_mtx_);
    
    if (!access_log_file_ || !access_log_file_->is_open()) {
        return false;
    }
    
    std::string timestamp = get_timestamp();
    *access_log_file_ << "[" << timestamp << "] " << message << std::endl;
    access_log_file_->flush();
    
    return true;
}

void Logger::set_level(LogLevel level) {
    std::lock_guard<std::mutex> lock(mtx_);
    current_level_ = level;
}

LogLevel Logger::get_level() const {
    std::lock_guard<std::mutex> lock(mtx_);
    return current_level_;
}

void Logger::flush() {
    std::lock_guard<std::mutex> lock(mtx_);
    if (log_file_ && log_file_->is_open()) {
        log_file_->flush();
    }
    if (access_log_file_ && access_log_file_->is_open()) {
        access_log_file_->flush();
    }
}

bool Logger::log(LogLevel level, const std::string& message) {
    if (level < current_level_) {
        return true; // Skip logging if level is below current
    }
    
    std::lock_guard<std::mutex> lock(mtx_);
    
    if (!log_file_ || !log_file_->is_open()) {
        return false;
    }
    
    // Check if we need to rotate the log file
    if (current_file_size_ > max_file_size_) {
        rotate_log_file();
    }
    
    std::string timestamp = get_timestamp();
    std::string level_str = get_level_string(level);
    
    std::string log_entry = "[" + timestamp + "] [" + level_str + "] " + message;
    *log_file_ << log_entry << std::endl;
    
    current_file_size_ += log_entry.length() + 1; // +1 for newline
    
    return true;
}

void Logger::rotate_log_file() {
    if (!log_file_ || !log_file_->is_open()) {
        return;
    }
    
    log_file_->close();
    
    // Create backup filename with timestamp
    std::time_t now = std::time(nullptr);
    std::stringstream ss;
    ss << log_file_path_ << "." << std::put_time(std::localtime(&now), "%Y%m%d_%H%M%S");
    std::string backup_path = ss.str();
    
    // Rename current log file to backup
    if (rename(log_file_path_.c_str(), backup_path.c_str()) != 0) {
        // If rename fails, just remove the old file
        std::remove(log_file_path_.c_str());
    }
    
    // Open new log file
    log_file_ = std::make_unique<std::ofstream>(log_file_path_, std::ios::app);
    current_file_size_ = 0;
}

std::string Logger::get_timestamp() const {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;
    
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    ss << '.' << std::setfill('0') << std::setw(3) << ms.count();
    
    return ss.str();
}

std::string Logger::get_level_string(LogLevel level) const {
    switch (level) {
        case LogLevel::DEBUG:   return "DEBUG";
        case LogLevel::INFO:    return "INFO";
        case LogLevel::WARNING: return "WARNING";
        case LogLevel::ERROR:   return "ERROR";
        case LogLevel::CRITICAL: return "CRITICAL";
        default:                return "UNKNOWN";
    }
}

} // namespace logging
} // namespace core
} // namespace aspire 