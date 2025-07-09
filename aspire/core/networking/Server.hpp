#ifndef ASPIRE_CORE_NETWORKING_SERVER_HPP
#define ASPIRE_CORE_NETWORKING_SERVER_HPP

#include <memory>
#include <unordered_map>
#include <atomic>
#include <sys/socket.h>
#include <netinet/in.h>

#include "EventLoop.hpp"
#include "Connection.hpp"

namespace aspire {
namespace core {
namespace networking {

/**
 * @brief High-performance HTTP server.
 * 
 * This class implements a complete HTTP server using epoll
 * for event-driven I/O with minimal dependencies.
 */
class Server {
public:
    /**
     * @brief Constructor.
     * @param port Port to listen on.
     * @param backlog Connection backlog size.
     */
    Server(int port = 8080, int backlog = 100);

    /**
     * @brief Destructor.
     */
    ~Server();

    /**
     * @brief Initialize the server.
     * @return true if successful, false otherwise.
     */
    bool initialize();

    /**
     * @brief Start the server.
     * @return true if successful, false otherwise.
     */
    bool start();

    /**
     * @brief Stop the server.
     */
    void stop();

    /**
     * @brief Check if server is running.
     * @return true if running, false otherwise.
     */
    bool is_running() const { return running_; }

    /**
     * @brief Get server statistics.
     * @return Statistics string.
     */
    std::string get_stats() const;

    /**
     * @brief Set request handler callback.
     * @param handler Request handler function.
     */
    void set_request_handler(std::function<void(std::shared_ptr<Connection>)> handler);

private:
    int port_;
    int backlog_;
    int listen_fd_;
    std::atomic<bool> running_;
    
    // Event loop
    std::unique_ptr<EventLoop> event_loop_;
    
    // Connections
    std::unordered_map<int, std::shared_ptr<Connection>> connections_;
    mutable std::mutex connections_mutex_;
    
    // Request handler
    std::function<void(std::shared_ptr<Connection>)> request_handler_;
    
    // Statistics
    size_t connections_accepted_;
    size_t connections_closed_;
    
    /**
     * @brief Create listening socket.
     * @return true if successful, false otherwise.
     */
    bool create_listen_socket();
    
    /**
     * @brief Handle new connection.
     * @param fd File descriptor of new connection.
     */
    void handle_new_connection(int fd);
    
    /**
     * @brief Handle connection read event.
     * @param fd File descriptor of connection.
     */
    void handle_connection_read(int fd);
    
    /**
     * @brief Handle connection write event.
     * @param fd File descriptor of connection.
     */
    void handle_connection_write(int fd);
    
    /**
     * @brief Handle connection error.
     * @param fd File descriptor of connection.
     */
    void handle_connection_error(int fd);
    
    /**
     * @brief Accept new connection.
     * @param client_addr Reference to sockaddr_in to store client address.
     * @return File descriptor of new connection, -1 on error.
     */
    int accept_connection(struct sockaddr_in& client_addr);
    
    /**
     * @brief Close connection.
     * @param fd File descriptor to close.
     */
    void close_connection(int fd);
};

} // namespace networking
} // namespace core
} // namespace aspire

#endif // ASPIRE_CORE_NETWORKING_SERVER_HPP 