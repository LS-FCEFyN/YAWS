#pragma once

/**
 * @class Client
 * @brief Represents a single client connection accepted by the Server.
 *
 * Owns the connected socket's file descriptor for the lifetime of the
 * connection and closes it automatically on destruction.
 */
class Client
{
public:
    /**
     * @brief Constructs a Client wrapping an already-accepted socket.
     * @param socket_fd File descriptor returned by accept().
     */
    explicit Client(int socket_fd);

    /**
     * @brief Destructor. Closes socket_fd if still owned.
     */
    ~Client();

    /**
     * @brief Returns the underlying socket file descriptor.
     */
    int get_socket_fd() const;

    // Client owns a unique OS resource (the socket), so it can't be copied —
    // but unlike Server, it's not a singleton, so moving it (e.g. into a
    // ThreadPool task) is fine and intentionally left enabled.
    Client(const Client &) = delete;
    Client &operator=(const Client &) = delete;
    Client(Client &&other) noexcept;
    Client &operator=(Client &&other) noexcept;

private:
    /// Connected socket file descriptor, or -1 once moved-from/closed.
    int socket_fd;
};