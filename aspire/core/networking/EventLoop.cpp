#include "EventLoop.hpp"
#include <unistd.h>
#include <errno.h>
#include <sstream>
#include <cstring>

namespace aspire {
namespace core {
namespace networking {

EventLoop::EventLoop(int max_events, int timeout_ms)
    : epoll_fd_(-1)
    , max_events_(max_events)
    , timeout_ms_(timeout_ms)
    , running_(false)
    , events_processed_(0)
    , callbacks_executed_(0) {
}

EventLoop::~EventLoop() {
    stop();
    if (epoll_fd_ >= 0) {
        close(epoll_fd_);
        epoll_fd_ = -1;
    }
}

bool EventLoop::initialize() {
    epoll_fd_ = epoll_create1(EPOLL_CLOEXEC);
    if (epoll_fd_ < 0) {
        return false;
    }
    
    return true;
}

bool EventLoop::add_fd(int fd, EventType events, EventCallback callback) {
    if (epoll_fd_ < 0) {
        return false;
    }
    
    struct epoll_event event;
    std::memset(&event, 0, sizeof(event));
    event.events = static_cast<uint32_t>(events);
    event.data.fd = fd;
    
    if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, fd, &event) < 0) {
        return false;
    }
    
    {
        std::lock_guard<std::mutex> lock(callbacks_mutex_);
        callbacks_[fd] = callback;
    }
    
    return true;
}

bool EventLoop::remove_fd(int fd) {
    if (epoll_fd_ < 0) {
        return false;
    }
    
    if (epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, fd, nullptr) < 0) {
        return false;
    }
    
    {
        std::lock_guard<std::mutex> lock(callbacks_mutex_);
        callbacks_.erase(fd);
    }
    
    return true;
}

bool EventLoop::modify_fd(int fd, EventType events) {
    if (epoll_fd_ < 0) {
        return false;
    }
    
    struct epoll_event event;
    std::memset(&event, 0, sizeof(event));
    event.events = static_cast<uint32_t>(events);
    event.data.fd = fd;
    
    if (epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, fd, &event) < 0) {
        return false;
    }
    
    return true;
}

bool EventLoop::run() {
    if (epoll_fd_ < 0) {
        return false;
    }
    
    running_ = true;
    
    std::vector<struct epoll_event> events(max_events_);
    
    while (running_) {
        int num_events = epoll_wait(epoll_fd_, events.data(), max_events_, timeout_ms_);
        
        if (num_events < 0) {
            if (errno == EINTR) {
                // Interrupted by signal, continue
                continue;
            } else {
                // Error occurred
                running_ = false;
                return false;
            }
        } else if (num_events == 0) {
            // Timeout, continue
            continue;
        }
        
        // Process events
        process_events(events.data(), num_events);
    }
    
    return true;
}

void EventLoop::stop() {
    running_ = false;
}

void EventLoop::process_events(struct epoll_event* events, int num_events) {
    events_processed_ += num_events;
    
    for (int i = 0; i < num_events; ++i) {
        int fd = events[i].data.fd;
        uint32_t event_flags = events[i].events;
        
        // Check for errors first
        if (event_flags & EPOLLERR || event_flags & EPOLLHUP) {
            handle_error(fd);
            continue;
        }
        
        // Find and execute callback
        EventCallback callback;
        {
            std::lock_guard<std::mutex> lock(callbacks_mutex_);
            auto it = callbacks_.find(fd);
            if (it != callbacks_.end()) {
                callback = it->second;
            }
        }
        
        if (callback) {
            EventType event_type = EventType::READ;
            if (event_flags & EPOLLOUT) {
                event_type = EventType::WRITE;
            }
            if (event_flags & EPOLLIN && event_flags & EPOLLOUT) {
                event_type = EventType::READ_WRITE;
            }
            
            try {
                callback(fd, event_type);
                callbacks_executed_++;
            } catch (...) {
                // Handle callback exceptions
                handle_error(fd);
            }
        }
    }
}

void EventLoop::handle_error(int fd) {
    // Remove the file descriptor from epoll
    epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, fd, nullptr);
    
    // Remove callback
    {
        std::lock_guard<std::mutex> lock(callbacks_mutex_);
        callbacks_.erase(fd);
    }
    
    // Close the file descriptor
    close(fd);
}

std::string EventLoop::get_stats() const {
    std::ostringstream oss;
    oss << "EventLoop[epoll_fd=" << epoll_fd_ << "] "
        << "Running: " << (running_ ? "true" : "false") << " "
        << "Events Processed: " << events_processed_ << " "
        << "Callbacks Executed: " << callbacks_executed_ << " "
        << "Active Callbacks: " << callbacks_.size();
    return oss.str();
}

} // namespace networking
} // namespace core
} // namespace aspire 