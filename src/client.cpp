#include <utility>

// Project-specific headers
#include "client.hpp"

// System headers
#include <unistd.h>

Client::Client(int socket_fd) : socket_fd(socket_fd) {}

Client::~Client()
{
    if (socket_fd != -1) close(socket_fd);
}

int Client::get_socket_fd() const { return socket_fd; }

Client::Client(Client &&other) noexcept : socket_fd(std::exchange(other.socket_fd, -1)) {}

Client &Client::operator=(Client &&other) noexcept
{
    if (this != &other)
    {
        if (socket_fd != -1) close(socket_fd);
        socket_fd = std::exchange(other.socket_fd, -1);
    }
    return *this;
}