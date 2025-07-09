#include "Server.hpp"
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sstream>
#include <cstring>

namespace aspire {
namespace core {
namespace networking {

Server::Server(int port, int backlog)
    : port_(port)
    , backlog_(backlog)
    , listen_fd_(-1)
    , running_(false)
    , connections_accepted_(0)
    , connections_closed_(0) {
    
    event_loop_ = std::make_unique<EventLoop>();
}

Server::~Server() {
    stop();
}

bool Server::initialize() {
    if (!event_loop_->initialize()) {
        return false;
    }
    
    if (!create_listen_socket()) {
        return false;
    }
    
    // Add listen socket to event loop
    if (!event_loop_->add_fd(listen_fd_, EventType::READ, 
        [this](int fd, EventType events) {
            handle_new_connection(fd);
        })) {
        return false;
    }
    
    return true;
}

bool Server::start() {
    if (listen_fd_ < 0) {
        return false;
    }
    
    running_ = true;
    
    // Run the event loop
    return event_loop_->run();
}

void Server::stop() {
    running_ = false;
    
    // Close all connections
    {
        std::lock_guard<std::mutex> lock(connections_mutex_);
        for (auto& pair : connections_) {
            pair.second->close_connection();
        }
        connections_.clear();
    }
    
    // Stop event loop
    if (event_loop_) {
        event_loop_->stop();
    }
    
    // Close listen socket
    if (listen_fd_ >= 0) {
        close(listen_fd_);
        listen_fd_ = -1;
    }
}

std::string Server::get_stats() const {
    std::ostringstream oss;
    oss << "Server[port=" << port_ << "] "
        << "Running: " << (running_ ? "true" : "false") << " "
        << "Connections Accepted: " << connections_accepted_ << " "
        << "Connections Closed: " << connections_closed_ << " "
        << "Active Connections: " << connections_.size();
    
    if (event_loop_) {
        oss << " " << event_loop_->get_stats();
    }
    
    return oss.str();
}

void Server::set_request_handler(std::function<void(std::shared_ptr<Connection>)> handler) {
    request_handler_ = handler;
}

bool Server::create_listen_socket() {
    // Create socket
    listen_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd_ < 0) {
        return false;
    }
    
    // Set socket options
    int flag = 1;
    if (setsockopt(listen_fd_, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag)) < 0) {
        close(listen_fd_);
        listen_fd_ = -1;
        return false;
    }
    
    // Set non-blocking
    int flags = fcntl(listen_fd_, F_GETFL, 0);
    fcntl(listen_fd_, F_SETFL, flags | O_NONBLOCK);
    
    // Bind socket
    struct sockaddr_in server_addr;
    std::memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port_);
    
    if (bind(listen_fd_, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        close(listen_fd_);
        listen_fd_ = -1;
        return false;
    }
    
    // Listen
    if (listen(listen_fd_, backlog_) < 0) {
        close(listen_fd_);
        listen_fd_ = -1;
        return false;
    }
    
    return true;
}

void Server::handle_new_connection(int fd) {
    struct sockaddr_in client_addr;
    int client_fd = accept_connection(client_addr);
    if (client_fd < 0) {
        return;
    }
    
    // Create connection object
    auto connection = std::make_shared<Connection>(client_fd, client_addr);
    
    // Add to connections map
    {
        std::lock_guard<std::mutex> lock(connections_mutex_);
        connections_[client_fd] = connection;
    }
    
    // Add to event loop
    event_loop_->add_fd(client_fd, EventType::READ,
        [this](int fd, EventType events) {
            if (events == EventType::READ) {
                handle_connection_read(fd);
            } else if (events == EventType::WRITE) {
                handle_connection_write(fd);
            } else if (events == EventType::READ_WRITE) {
                handle_connection_read(fd);
                handle_connection_write(fd);
            }
        });
    
    connections_accepted_++;
}

void Server::handle_connection_read(int fd) {
    std::shared_ptr<Connection> connection;
    {
        std::lock_guard<std::mutex> lock(connections_mutex_);
        auto it = connections_.find(fd);
        if (it == connections_.end()) {
            return;
        }
        connection = it->second;
    }
    
    if (!connection || !connection->is_active()) {
        close_connection(fd);
        return;
    }
    
    // Read data from connection
    char buffer[4096];
    ssize_t bytes_read = connection->read_data(buffer, sizeof(buffer));
    
    if (bytes_read > 0) {
        // Call request handler if available
        if (request_handler_) {
            request_handler_(connection);
        }
    } else if (bytes_read == 0) {
        // Connection closed by client
        close_connection(fd);
    } else if (bytes_read < 0) {
        // Error occurred
        close_connection(fd);
    }
}

void Server::handle_connection_write(int fd) {
    std::shared_ptr<Connection> connection;
    {
        std::lock_guard<std::mutex> lock(connections_mutex_);
        auto it = connections_.find(fd);
        if (it == connections_.end()) {
            return;
        }
        connection = it->second;
    }
    
    if (!connection || !connection->is_active()) {
        close_connection(fd);
        return;
    }
    
    // Handle write events if needed
    // This is where you would handle pending write operations
}

void Server::handle_connection_error(int fd) {
    close_connection(fd);
}

int Server::accept_connection(struct sockaddr_in& client_addr) {
    socklen_t client_addr_len = sizeof(client_addr);
    int client_fd = accept(listen_fd_, (struct sockaddr*)&client_addr, &client_addr_len);
    if (client_fd < 0) {
        return -1;
    }
    int flags = fcntl(client_fd, F_GETFL, 0);
    fcntl(client_fd, F_SETFL, flags | O_NONBLOCK);
    return client_fd;
}

void Server::close_connection(int fd) {
    // Remove from event loop
    event_loop_->remove_fd(fd);
    
    // Remove from connections map
    {
        std::lock_guard<std::mutex> lock(connections_mutex_);
        auto it = connections_.find(fd);
        if (it != connections_.end()) {
            it->second->close_connection();
            connections_.erase(it);
        }
    }
    
    connections_closed_++;
}

} // namespace networking
} // namespace core
} // namespace aspire 