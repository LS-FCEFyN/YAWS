// Standard library headers
#include <charconv>
#include <chrono>
#include <cerrno>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <optional>
#include <ranges>
#include <stdexcept>
#include <string_view>
#include <utility>
#include <vector>

// Project-specific headers
#include "server.hpp"
#include "strategy/misc/server_capacity_gate.hpp"

// System headers
#include <netdb.h>
#include <poll.h>
#include <sys/sendfile.h>
#include <sys/socket.h>
#include <unistd.h>

// GCC Extensions
#include <ext/stdio_filebuf.h>

namespace
{
bool write_all(int socket_fd, const char *data, size_t size)
{
    while (size > 0)
    {
        const ssize_t bytes_written = write(socket_fd, data, size);
        if (bytes_written < 0)
        {
            if (errno == EINTR)
                continue;
            return false;
        }
        if (bytes_written == 0)
            return false;
        data += bytes_written;
        size -= static_cast<size_t>(bytes_written);
    }
    return true;
}

bool send_file_all(int socket_fd, int file_fd, off_t offset, size_t size)
{
    while (size > 0)
    {
        const ssize_t bytes_sent = sendfile(socket_fd, file_fd, &offset, size);
        if (bytes_sent < 0)
        {
            if (errno == EINTR)
                continue;
            return false;
        }
        if (bytes_sent == 0)
            return false;
        size -= static_cast<size_t>(bytes_sent);
    }
    return true;
}

bool is_within_root(const std::filesystem::path &candidate, const std::filesystem::path &root)
{
    auto root_it = root.begin();
    auto candidate_it = candidate.begin();
    for (; root_it != root.end() && candidate_it != candidate.end(); ++root_it, ++candidate_it)
    {
        if (*root_it != *candidate_it)
            return false;
    }
    return root_it == root.end();
}

bool parse_range_header(std::string_view value, long &start, long &end)
{
    constexpr std::string_view prefix = "bytes=";
    if (!value.starts_with(prefix))
        return false;

    const std::string_view range = value.substr(prefix.size());
    const size_t dash = range.find('-');
    if (dash == std::string_view::npos || dash == 0 || range.find('-', dash + 1) != std::string_view::npos)
        return false;

    const auto parse_number = [](std::string_view text, long &result) {
        const auto parsed = std::from_chars(text.data(), text.data() + text.size(), result);
        if (parsed.ec == std::errc::result_out_of_range)
        {
            result = std::numeric_limits<long>::max();
            return true;
        }
        return parsed.ec == std::errc{} && parsed.ptr == text.data() + text.size();
    };

    if (!parse_number(range.substr(0, dash), start))
        return false;

    const std::string_view end_text = range.substr(dash + 1);
    if (end_text.empty())
    {
        end = -1;
        return true;
    }

    return parse_number(end_text, end);
}
}

std::unique_ptr<Server> Server::instance = nullptr;

void Server::start()
{
    // Lazily construct the ThreadPool if it hasn't been created yet.
    if (!this->pool)
    {
        this->pool = std::make_unique<ThreadPool>(this->max_connections);
    }

    // Set the running flag to true and enter the accept loop.
    this->running = true;
    this->accept_loop();
}

void Server::stop()
{
    // Set the running flag to false, which will cause the accept loop to exit.
    this->running = false;

    // Close the listening socket to unblock the accept() call.
    close(this->socket_fd);

    // Tear down the ThreadPool, which will wait for all in-flight tasks to finish.
    this->pool.reset();
}

Server &Server::get_instance(std::optional<std::string> port, int max_connections)
{
    // If the singleton instance hasn't been created yet, construct it.
    if (!instance)
    {
        instance = std::unique_ptr<Server>(new Server(port, max_connections));
    }
    return *instance;
}

void Server::attach(std::shared_ptr<ConnectionObserver> observer)
{
    this->observers.push_back(std::move(observer));
}

void Server::set_overload_strategy(std::unique_ptr<OverloadStrategy> strategy)
{
    this->overload_strategy = std::move(strategy);
}

Server::Server(std::optional<std::string> port, int max_connections)
    : port(port.value_or("8080"))
{
    // Initialize the hints structure to zero and set the desired socket properties.
    struct addrinfo hints = {}, *bind_address;

    // AF_INET is the address family for IPv4.
    hints.ai_family = AF_INET;
    // SOCK_STREAM is the socket type for TCP.
    hints.ai_socktype = SOCK_STREAM;
    // AI_PASSIVE indicates that the socket will be used for binding, not connecting.
    hints.ai_flags = AI_PASSIVE;

    // Get the address information for the specified port, port defaults to 8080 if none is specified.
    if (getaddrinfo(nullptr, this->port.value().c_str(), &hints, &bind_address) != 0)
    {
        throw std::runtime_error("getaddrinfo failed");
    }

    // Create a socket using the parameters from bind_address.
    this->socket_fd = socket(bind_address->ai_family, bind_address->ai_socktype,
                             bind_address->ai_protocol);

    if (this->socket_fd == -1)
    {
        throw std::runtime_error("socket() failed");
    }

    // C++ doesn't have compound literals like C does, so a named const is required here.
    const int enable = 1;

    if (setsockopt(this->socket_fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
    {
        throw std::runtime_error("setsockopt(SO_REUSEADDR) failed");
    }

    // The server may close and restart quickly, so we set SO_REUSEPORT to allow multiple binds to the same port.
    // This will be used as a conditional compilation flag to ensure compatibility with systems that support it.
#ifdef SO_REUSEPORT
    if (setsockopt(this->socket_fd, SOL_SOCKET, SO_REUSEPORT, &enable, sizeof(int)) < 0)
    {
        throw std::runtime_error("setsockopt(SO_REUSEPORT) failed");
    }
#endif

    // Finally we bind the socket to the specified port and address.
    // If this fails, it usually means the port is already in use, or insufficient permissions
    // to bind to the port (e.g., ports < 1024 require root privileges).
    if (bind(this->socket_fd, bind_address->ai_addr, bind_address->ai_addrlen) < 0)
    {
        throw std::runtime_error("bind() failed");
    }

    // Free the memory allocated for bind_address by getaddrinfo.
    freeaddrinfo(bind_address);

    // Store the maximum number of concurrent connections for later use in listen(),
    // and also to initialize the ThreadPool with the same number of worker threads.
    this->max_connections = max_connections;

    if (listen(this->socket_fd, this->max_connections) < 0)
    {
        throw std::runtime_error("listen() failed");
    }
}

Server::~Server()
{
    // Close the listening socket file descriptor.
    close(this->socket_fd);
}

void Server::accept_loop()
{
    while (this->running)
    {
        sockaddr_storage client_addr{};
        socklen_t client_len = sizeof(client_addr);

        const int client_fd = accept(this->socket_fd, reinterpret_cast<sockaddr *>(&client_addr), &client_len);

        if (client_fd < 0)
        {
            if (!this->running) break; // stop() closed the socket -- clean shutdown
            continue;
        }

        auto client = std::make_shared<Client>(client_fd);

        for (const auto &observer : this->observers)
        {
            observer->on_connection_accepted(*client);
        }

        bool at_capacity;
        {
            std::lock_guard<std::mutex> lock(this->capacity_mutex);
            at_capacity = (this->active_connections >= this->max_connections);
        }

        if (!at_capacity)
        {
            this->admit_connection(client);
        }
        else if (this->overload_strategy)
        {
            ServerCapacityGate gate(this->capacity_mutex, this->capacity_cv, this->active_connections, this->max_connections);
            this->overload_strategy->handle_overload(
                client, [this](std::shared_ptr<Client> c) { this->admit_connection(std::move(c)); }, gate);
        }
        else
        {
            // No strategy configured: preserve the original unbounded behavior.
            this->admit_connection(client);
        }
    }
}

void Server::admit_connection(std::shared_ptr<Client> client)
{
    {
        std::lock_guard<std::mutex> lock(this->capacity_mutex);
        ++this->active_connections;
    }

    this->pool->enqueue_task([this, client] {
        this->handle_connection(client);
        {
            std::lock_guard<std::mutex> lock(this->capacity_mutex);
            --this->active_connections;
        }
        this->capacity_cv.notify_one();
    });
}

void Server::handle_connection(std::shared_ptr<Client> client)
{
    static constexpr size_t max_header_bytes = 16 * 1024;
    static constexpr auto header_timeout = std::chrono::seconds(10);
    std::string buffer;
    char chunk[4096];
    const auto header_deadline = std::chrono::steady_clock::now() + header_timeout;

    while (buffer.find("\r\n\r\n") == std::string::npos)
    {
        const auto remaining = std::chrono::duration_cast<std::chrono::milliseconds>(
            header_deadline - std::chrono::steady_clock::now());
        if (remaining.count() <= 0)
            return;

        struct pollfd poll_descriptor{
            client->get_socket_fd(), POLLIN, 0};
        const int poll_result = poll(&poll_descriptor, 1, static_cast<int>(remaining.count()));
        if (poll_result <= 0)
            return;

        const ssize_t bytes_read = recv(client->get_socket_fd(), chunk, sizeof(chunk), 0);
        if (bytes_read <= 0) return;
        buffer.append(chunk, static_cast<size_t>(bytes_read));
        if (buffer.size() > max_header_bytes)
        {
            this->send_error(client, StatusCode::BadRequest);
            return;
        }
    }

    const auto request = Request::parse(buffer);
    if (!request) { this->send_error(client, StatusCode::BadRequest); return; }
    if (!request->method()) { this->send_error(client, StatusCode::NotImplemented); return; }

    this->serve_file(client, *request);
}

void Server::serve_file(const std::shared_ptr<Client> &client, const Request &request)
{
    const std::filesystem::path public_root = std::filesystem::weakly_canonical("public");
    const std::string_view target = request.target();
    const std::filesystem::path relative_target = target.starts_with('/') ?
        std::filesystem::path(target.substr(1)) : std::filesystem::path(target);
    const std::filesystem::path requested_path = public_root / relative_target;
    const std::filesystem::path absolute_path = std::filesystem::weakly_canonical(requested_path);

    if (!is_within_root(absolute_path, public_root) ||
        !std::filesystem::exists(absolute_path) || std::filesystem::is_directory(absolute_path))
    {
        this->send_error(client, StatusCode::NotFound);
        return;
    }

    std::ifstream file(absolute_path, std::ios::binary);
    if (!file.is_open()) { this->send_error(client, StatusCode::InternalServerError); return; }

    const int file_fd = static_cast<__gnu_cxx::stdio_filebuf<char> *>(file.rdbuf())->fd();
    file.seekg(0, std::ios::end);
    const long file_size = file.tellg();
    file.seekg(0, std::ios::beg);

    const bool is_head = (request.method() == Method::Head);
    const auto range_header = request.header("Range");

    if (range_header)
    {
        long start = 0, end = -1;
        if (!parse_range_header(*range_header, start, end))
        {
            this->send_error(client, StatusCode::BadRequest);
            return;
        }

        if (start < 0 || (end >= 0 && end < start) || start >= file_size)
        {
            this->send_error(client, StatusCode::RangeNotSatisfiable);
            return;
        }
        if (end == -1 || end >= file_size) end = file_size - 1;

        const size_t content_length = static_cast<size_t>(end - start + 1);

        Response response(StatusCode::PartialContent);
        response.set_header("Connection", "close");
        response.set_header("Content-Type", std::string(mime_type_for(request.target())));
        response.set_header("Content-Length", std::to_string(content_length));
        response.set_header("Content-Range", "bytes " + std::to_string(start) + "-" + std::to_string(end) +
                                                  "/" + std::to_string(file_size));

        const std::string headers = response.serialize_headers();
        if (!write_all(client->get_socket_fd(), headers.data(), headers.size()))
            return;

        if (!is_head)
        {
            send_file_all(client->get_socket_fd(), file_fd, start, content_length);
        }
        return;
    }

    Response response(StatusCode::Ok);
    response.set_header("Connection", "close");
    response.set_header("Content-Type", std::string(mime_type_for(request.target())));
    response.set_header("Content-Length", std::to_string(file_size));
    response.set_header("Accept-Ranges", "bytes");

    const std::string headers = response.serialize_headers();
    if (!write_all(client->get_socket_fd(), headers.data(), headers.size()))
        return;

    if (!is_head)
    {
        send_file_all(client->get_socket_fd(), file_fd, 0, static_cast<size_t>(file_size));
    }
}

void Server::send_error(const std::shared_ptr<Client> &client, StatusCode status)
{
    Response response(status);
    response.set_header("Connection", "close");
    response.set_header("Content-Type", "text/plain");
    response.set_body(std::string(reason_phrase(status)));

    const std::string headers = response.serialize_headers();
    if (!write_all(client->get_socket_fd(), headers.data(), headers.size()))
        return;

    if (response.body())
    {
        write_all(client->get_socket_fd(), response.body()->data(), response.body()->size());
    }
}