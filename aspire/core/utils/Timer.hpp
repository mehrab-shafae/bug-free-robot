#ifndef ASPIRE_CORE_UTILS_TIMER_HPP
#define ASPIRE_CORE_UTILS_TIMER_HPP

#include <chrono>
#include <string>

namespace aspire {
namespace core {
namespace utils {

/**
 * @brief High-precision timer utility for performance measurement.
 * 
 * This class provides precise timing capabilities for measuring
 * elapsed time in performance-critical operations.
 */
class Timer {
public:
    /**
     * @brief Constructor initializes timer in stopped state.
     */
    Timer();

    /**
     * @brief Start the timer.
     */
    void start();

    /**
     * @brief Stop the timer.
     */
    void stop();

    /**
     * @brief Reset the timer to initial state.
     */
    void reset();

    /**
     * @brief Get elapsed time in seconds.
     * @return Elapsed time in seconds as double.
     */
    double elapsed_seconds() const;

    /**
     * @brief Get elapsed time in milliseconds.
     * @return Elapsed time in milliseconds as double.
     */
    double elapsed_milliseconds() const;

    /**
     * @brief Get elapsed time in microseconds.
     * @return Elapsed time in microseconds as double.
     */
    double elapsed_microseconds() const;

    /**
     * @brief Get elapsed time in nanoseconds.
     * @return Elapsed time in nanoseconds as double.
     */
    double elapsed_nanoseconds() const;

    /**
     * @brief Check if timer is running.
     * @return true if timer is running, false otherwise.
     */
    bool is_running() const;

    /**
     * @brief Get formatted elapsed time string.
     * @return Formatted time string.
     */
    std::string get_formatted_time() const;

private:
    using Clock = std::chrono::steady_clock;
    using TimePoint = std::chrono::time_point<Clock>;
    using Duration = std::chrono::duration<double>;

    TimePoint start_time_;
    TimePoint end_time_;
    bool running_;
    bool has_ended_;
};

} // namespace utils
} // namespace core
} // namespace aspire

#endif // ASPIRE_CORE_UTILS_TIMER_HPP 