#ifndef ASPIRE_CORE_NETWORKING_CONNECTION_HPP
#define ASPIRE_CORE_NETWORKING_CONNECTION_HPP

#include <string>
#include <vector>
#include <memory>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

namespace aspire {
namespace core {
namespace networking {

/**
 * @brief Connection state enumeration.
 */
enum class ConnectionState {
    CONNECTING,
    CONNECTED,
    READING,
    WRITING,
    CLOSING,
    CLOSED
};

/**
 * @brief Network connection manager.
 * 
 * This class manages individual network connections with
 * optimized buffer handling and Linux-specific features.
 */
class Connection {
public:
    /**
     * @brief Constructor.
     * @param socket_fd Socket file descriptor.
     * @param client_addr Client address information.
     */
    Connection(int socket_fd, const struct sockaddr_in& client_addr);

    /**
     * @brief Destructor.
     */
    ~Connection();

    /**
     * @brief Get socket file descriptor.
     * @return Socket file descriptor.
     */
    int get_socket_fd() const { return socket_fd_; }

    /**
     * @brief Get client IP address as string.
     * @return Client IP address.
     */
    std::string get_client_ip() const;

    /**
     * @brief Get client port.
     * @return Client port number.
     */
    uint16_t get_client_port() const;

    /**
     * @brief Get connection state.
     * @return Current connection state.
     */
    ConnectionState get_state() const { return state_; }

    /**
     * @brief Set connection state.
     * @param state New connection state.
     */
    void set_state(ConnectionState state) { state_ = state; }

    /**
     * @brief Read data from connection.
     * @param buffer Buffer to store read data.
     * @param max_size Maximum bytes to read.
     * @return Number of bytes read, -1 on error.
     */
    ssize_t read_data(char* buffer, size_t max_size);

    /**
     * @brief Write data to connection.
     * @param data Data to write.
     * @param size Size of data to write.
     * @return Number of bytes written, -1 on error.
     */
    ssize_t write_data(const char* data, size_t size);

    /**
     * @brief Write data to connection using sendfile.
     * @param file_fd File descriptor to send.
     * @param offset File offset to start from.
     * @param count Number of bytes to send.
     * @return Number of bytes sent, -1 on error.
     */
    ssize_t send_file(int file_fd, off_t offset, size_t count);

    /**
     * @brief Close the connection.
     * @return true if successful, false otherwise.
     */
    bool close_connection();

    /**
     * @brief Check if connection is active.
     * @return true if connection is active, false otherwise.
     */
    bool is_active() const { return state_ != ConnectionState::CLOSED; }

    /**
     * @brief Get connection statistics.
     * @return Connection statistics string.
     */
    std::string get_stats() const;

    /**
     * @brief Set socket options for performance.
     * @return true if successful, false otherwise.
     */
    bool optimize_socket();

private:
    int socket_fd_;
    struct sockaddr_in client_addr_;
    ConnectionState state_;
    
    // Statistics
    size_t bytes_read_;
    size_t bytes_written_;
    size_t requests_handled_;
    
    // Buffer management
    static constexpr size_t BUFFER_SIZE = 8192;
    std::vector<char> read_buffer_;
    std::vector<char> write_buffer_;
};

} // namespace networking
} // namespace core
} // namespace aspire

#endif // ASPIRE_CORE_NETWORKING_CONNECTION_HPP 