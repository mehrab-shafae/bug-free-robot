#include "Connection.hpp"
#include <unistd.h>
#include <fcntl.h>
#include <sys/sendfile.h>
#include <errno.h>
#include <sstream>
#include <cstring>
#include <netinet/tcp.h>

namespace aspire {
namespace core {
namespace networking {

Connection::Connection(int socket_fd, const struct sockaddr_in& client_addr)
    : socket_fd_(socket_fd)
    , client_addr_(client_addr)
    , state_(ConnectionState::CONNECTED)
    , bytes_read_(0)
    , bytes_written_(0)
    , requests_handled_(0)
    , read_buffer_(BUFFER_SIZE)
    , write_buffer_(BUFFER_SIZE) {
    
    // Set socket to non-blocking mode
    int flags = fcntl(socket_fd_, F_GETFL, 0);
    fcntl(socket_fd_, F_SETFL, flags | O_NONBLOCK);
    
    // Optimize socket for performance
    optimize_socket();
}

Connection::~Connection() {
    close_connection();
}

std::string Connection::get_client_ip() const {
    char ip_str[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &client_addr_.sin_addr, ip_str, INET_ADDRSTRLEN);
    return std::string(ip_str);
}

uint16_t Connection::get_client_port() const {
    return ntohs(client_addr_.sin_port);
}

ssize_t Connection::read_data(char* buffer, size_t max_size) {
    if (state_ == ConnectionState::CLOSED) {
        return -1;
    }
    
    ssize_t bytes_read = recv(socket_fd_, buffer, max_size, MSG_NOSIGNAL);
    
    if (bytes_read > 0) {
        bytes_read_ += bytes_read;
        state_ = ConnectionState::READING;
    } else if (bytes_read == 0) {
        // Connection closed by client
        state_ = ConnectionState::CLOSING;
    } else if (bytes_read < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            // No data available (non-blocking)
            return 0;
        } else {
            // Error occurred
            state_ = ConnectionState::CLOSING;
        }
    }
    
    return bytes_read;
}

ssize_t Connection::write_data(const char* data, size_t size) {
    if (state_ == ConnectionState::CLOSED) {
        return -1;
    }
    
    ssize_t bytes_written = send(socket_fd_, data, size, MSG_NOSIGNAL);
    
    if (bytes_written > 0) {
        bytes_written_ += bytes_written;
        state_ = ConnectionState::WRITING;
    } else if (bytes_written < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            // Would block (non-blocking)
            return 0;
        } else {
            // Error occurred
            state_ = ConnectionState::CLOSING;
        }
    }
    
    return bytes_written;
}

ssize_t Connection::send_file(int file_fd, off_t offset, size_t count) {
    if (state_ == ConnectionState::CLOSED) {
        return -1;
    }
    
    ssize_t bytes_sent = sendfile(socket_fd_, file_fd, &offset, count);
    
    if (bytes_sent > 0) {
        bytes_written_ += bytes_sent;
        state_ = ConnectionState::WRITING;
    } else if (bytes_sent < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            // Would block (non-blocking)
            return 0;
        } else {
            // Error occurred
            state_ = ConnectionState::CLOSING;
        }
    }
    
    return bytes_sent;
}

bool Connection::close_connection() {
    if (state_ == ConnectionState::CLOSED) {
        return true;
    }
    
    state_ = ConnectionState::CLOSING;
    
    if (socket_fd_ >= 0) {
        close(socket_fd_);
        socket_fd_ = -1;
    }
    
    state_ = ConnectionState::CLOSED;
    return true;
}

std::string Connection::get_stats() const {
    std::ostringstream oss;
    oss << "Connection[" << get_client_ip() << ":" << get_client_port() << "] "
        << "State: " << static_cast<int>(state_) << " "
        << "Read: " << bytes_read_ << " bytes "
        << "Written: " << bytes_written_ << " bytes "
        << "Requests: " << requests_handled_;
    return oss.str();
}

bool Connection::optimize_socket() {
    if (socket_fd_ < 0) {
        return false;
    }
    
    // Set TCP_NODELAY to disable Nagle's algorithm
    int flag = 1;
    if (setsockopt(socket_fd_, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag)) < 0) {
        return false;
    }
    
    // Set SO_KEEPALIVE
    if (setsockopt(socket_fd_, SOL_SOCKET, SO_KEEPALIVE, &flag, sizeof(flag)) < 0) {
        return false;
    }
    
    // Set receive buffer size
    int rcvbuf_size = 64 * 1024; // 64KB
    if (setsockopt(socket_fd_, SOL_SOCKET, SO_RCVBUF, &rcvbuf_size, sizeof(rcvbuf_size)) < 0) {
        return false;
    }
    
    // Set send buffer size
    int sndbuf_size = 64 * 1024; // 64KB
    if (setsockopt(socket_fd_, SOL_SOCKET, SO_SNDBUF, &sndbuf_size, sizeof(sndbuf_size)) < 0) {
        return false;
    }
    
    // Set SO_REUSEADDR
    if (setsockopt(socket_fd_, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag)) < 0) {
        return false;
    }
    
    return true;
}

} // namespace networking
} // namespace core
} // namespace aspire 