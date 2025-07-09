#ifndef ASPIRE_CORE_CONFIG_CONFIG_HPP
#define ASPIRE_CORE_CONFIG_CONFIG_HPP

#include <string>
#include <map>
#include <memory>
#include <vector>

namespace aspire {
namespace core {
namespace config {

/**
 * @brief Singleton configuration manager for Aspire HTTP/2 server.
 * 
 * This class manages all configuration settings for the server including
 * network settings, performance tuning, and security parameters.
 */
class Config {
public:
    /**
     * @brief Get the singleton instance of Config.
     * @return Reference to the Config instance.
     */
    static Config& instance();

    /**
     * @brief Load configuration from file.
     * @param path Path to configuration file.
     * @return true if successful, false otherwise.
     */
    bool load(const std::string& path);

    /**
     * @brief Get string configuration value.
     * @param key Configuration key.
     * @param default_value Default value if key not found.
     * @return Configuration value as string.
     */
    std::string get_string(const std::string& key, 
                          const std::string& default_value = "") const;

    /**
     * @brief Get integer configuration value.
     * @param key Configuration key.
     * @param default_value Default value if key not found.
     * @return Configuration value as integer.
     */
    int get_int(const std::string& key, int default_value = 0) const;

    /**
     * @brief Get boolean configuration value.
     * @param key Configuration key.
     * @param default_value Default value if key not found.
     * @return Configuration value as boolean.
     */
    bool get_bool(const std::string& key, bool default_value = false) const;

    /**
     * @brief Set configuration value.
     * @param key Configuration key.
     * @param value Configuration value.
     */
    void set(const std::string& key, const std::string& value);

    /**
     * @brief Check if configuration key exists.
     * @param key Configuration key to check.
     * @return true if key exists, false otherwise.
     */
    bool has_key(const std::string& key) const;

    /**
     * @brief Get all configuration keys.
     * @return Vector of configuration keys.
     */
    std::vector<std::string> get_keys() const;

private:
    Config() = default;
    ~Config() = default;
    Config(const Config&) = delete;
    Config& operator=(const Config&) = delete;

    std::map<std::string, std::string> data_;
    mutable std::mutex mtx_;
};

} // namespace config
} // namespace core
} // namespace aspire

#endif // ASPIRE_CORE_CONFIG_CONFIG_HPP 