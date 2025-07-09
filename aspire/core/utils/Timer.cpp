#include "Timer.hpp"
#include <sstream>
#include <iomanip>

namespace aspire {
namespace core {
namespace utils {

Timer::Timer() : running_(false), has_ended_(false) {
}

void Timer::start() {
    start_time_ = Clock::now();
    running_ = true;
    has_ended_ = false;
}

void Timer::stop() {
    if (running_) {
        end_time_ = Clock::now();
        running_ = false;
        has_ended_ = true;
    }
}

void Timer::reset() {
    running_ = false;
    has_ended_ = false;
}

double Timer::elapsed_seconds() const {
    if (!has_ended_ && !running_) {
        return 0.0;
    }
    
    TimePoint end = running_ ? Clock::now() : end_time_;
    Duration elapsed = end - start_time_;
    return elapsed.count();
}

double Timer::elapsed_milliseconds() const {
    return elapsed_seconds() * 1000.0;
}

double Timer::elapsed_microseconds() const {
    return elapsed_seconds() * 1000000.0;
}

double Timer::elapsed_nanoseconds() const {
    return elapsed_seconds() * 1000000000.0;
}

bool Timer::is_running() const {
    return running_;
}

std::string Timer::get_formatted_time() const {
    double seconds = elapsed_seconds();
    
    if (seconds < 0.001) {
        return std::to_string(static_cast<int>(seconds * 1000000)) + " μs";
    } else if (seconds < 1.0) {
        return std::to_string(static_cast<int>(seconds * 1000)) + " ms";
    } else {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(3) << seconds << " s";
        return oss.str();
    }
}

} // namespace utils
} // namespace core
} // namespace aspire 