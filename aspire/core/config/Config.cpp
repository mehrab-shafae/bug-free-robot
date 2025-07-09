#include "Config.hpp"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <mutex>

namespace aspire {
namespace core {
namespace config {

Config& Config::instance() {
    static Config instance;
    return instance;
}

bool Config::load(const std::string& path) {
    std::lock_guard<std::mutex> lock(mtx_);
    
    std::ifstream file(path);
    if (!file.is_open()) {
        return false;
    }

    std::string line;
    int line_number = 0;
    
    while (std::getline(file, line)) {
        line_number++;
        
        // Skip empty lines and comments
        if (line.empty() || line[0] == '#') {
            continue;
        }
        
        // Find the first '=' character
        size_t pos = line.find('=');
        if (pos == std::string::npos) {
            continue; // Skip lines without '='
        }
        
        // Extract key and value
        std::string key = line.substr(0, pos);
        std::string value = line.substr(pos + 1);
        
        // Trim whitespace from key and value
        key.erase(0, key.find_first_not_of(" \t"));
        key.erase(key.find_last_not_of(" \t") + 1);
        
        value.erase(0, value.find_first_not_of(" \t"));
        value.erase(value.find_last_not_of(" \t") + 1);
        
        if (!key.empty()) {
            data_[key] = value;
        }
    }
    
    return true;
}

std::string Config::get_string(const std::string& key, 
                              const std::string& default_value) const {
    std::lock_guard<std::mutex> lock(mtx_);
    
    auto it = data_.find(key);
    if (it != data_.end()) {
        return it->second;
    }
    return default_value;
}

int Config::get_int(const std::string& key, int default_value) const {
    std::string value = get_string(key, "");
    if (value.empty()) {
        return default_value;
    }
    
    try {
        return std::stoi(value);
    } catch (const std::exception&) {
        return default_value;
    }
}

bool Config::get_bool(const std::string& key, bool default_value) const {
    std::string value = get_string(key, "");
    if (value.empty()) {
        return default_value;
    }
    
    // Convert to lowercase for comparison
    std::transform(value.begin(), value.end(), value.begin(), ::tolower);
    
    return (value == "true" || value == "1" || value == "yes" || value == "on");
}

void Config::set(const std::string& key, const std::string& value) {
    std::lock_guard<std::mutex> lock(mtx_);
    data_[key] = value;
}

bool Config::has_key(const std::string& key) const {
    std::lock_guard<std::mutex> lock(mtx_);
    return data_.find(key) != data_.end();
}

std::vector<std::string> Config::get_keys() const {
    std::lock_guard<std::mutex> lock(mtx_);
    std::vector<std::string> keys;
    keys.reserve(data_.size());
    
    for (const auto& pair : data_) {
        keys.push_back(pair.first);
    }
    
    return keys;
}

} // namespace config
} // namespace core
} // namespace aspire 