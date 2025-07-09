#ifndef ASPIRE_CORE_NETWORKING_EVENTLOOP_HPP
#define ASPIRE_CORE_NETWORKING_EVENTLOOP_HPP

#include <memory>
#include <unordered_map>
#include <functional>
#include <sys/epoll.h>
#include <atomic>

#include "Connection.hpp"

namespace aspire {
namespace core {
namespace networking {

/**
 * @brief Event types for epoll.
 */
enum class EventType {
    READ = EPOLLIN,
    WRITE = EPOLLOUT,
    ERROR = EPOLLERR,
    HUP = EPOLLHUP,
    READ_WRITE = EPOLLIN | EPOLLOUT
};

/**
 * @brief Event callback function type.
 */
using EventCallback = std::function<void(int fd, EventType events)>;

/**
 * @brief High-performance epoll-based event loop.
 * 
 * This class implements a single-threaded event loop using epoll
 * for high-performance I/O multiplexing on Linux.
 */
class EventLoop {
public:
    /**
     * @brief Constructor.
     * @param max_events Maximum number of events to process per iteration.
     * @param timeout_ms Timeout for epoll_wait in milliseconds.
     */
    EventLoop(int max_events = 1024, int timeout_ms = 50);

    /**
     * @brief Destructor.
     */
    ~EventLoop();

    /**
     * @brief Initialize the event loop.
     * @return true if successful, false otherwise.
     */
    bool initialize();

    /**
     * @brief Add file descriptor to event loop.
     * @param fd File descriptor to add.
     * @param events Events to monitor.
     * @param callback Callback function for events.
     * @return true if successful, false otherwise.
     */
    bool add_fd(int fd, EventType events, EventCallback callback);

    /**
     * @brief Remove file descriptor from event loop.
     * @param fd File descriptor to remove.
     * @return true if successful, false otherwise.
     */
    bool remove_fd(int fd);

    /**
     * @brief Modify events for file descriptor.
     * @param fd File descriptor to modify.
     * @param events New events to monitor.
     * @return true if successful, false otherwise.
     */
    bool modify_fd(int fd, EventType events);

    /**
     * @brief Run the event loop.
     * @return true if successful, false otherwise.
     */
    bool run();

    /**
     * @brief Stop the event loop.
     */
    void stop();

    /**
     * @brief Check if event loop is running.
     * @return true if running, false otherwise.
     */
    bool is_running() const { return running_; }

    /**
     * @brief Get event loop statistics.
     * @return Statistics string.
     */
    std::string get_stats() const;

private:
    int epoll_fd_;
    int max_events_;
    int timeout_ms_;
    std::atomic<bool> running_;
    
    // Event callbacks
    std::unordered_map<int, EventCallback> callbacks_;
    mutable std::mutex callbacks_mutex_;
    
    // Statistics
    size_t events_processed_;
    size_t callbacks_executed_;
    
    /**
     * @brief Process epoll events.
     * @param events Array of epoll events.
     * @param num_events Number of events to process.
     */
    void process_events(struct epoll_event* events, int num_events);
    
    /**
     * @brief Handle epoll error.
     * @param fd File descriptor that caused error.
     */
    void handle_error(int fd);
};

} // namespace networking
} // namespace core
} // namespace aspire

#endif // ASPIRE_CORE_NETWORKING_EVENTLOOP_HPP 