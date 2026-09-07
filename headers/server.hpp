#pragma once

// Standard library headers
#include <atomic>
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

// Project-specific headers
#include "client.hpp"
#include "concurrency/threadpool.hpp"
#include "http/codes.hpp"
#include "http/mime.hpp"
#include "http/request.hpp"
#include "http/response.hpp"
#include "observer/connection_observer.hpp"
#include "strategy/overload_strategy.hpp"



/**
 * @class Server
 * @brief A singleton static HTTP server built directly on the UNIX sockets API.
 *
 * Accepts incoming TCP connections and dispatches each one as a task to a
 * fixed-size ThreadPool, which serves static files back to clients using
 * sendfile() for zero-copy transfer.
 */
class Server
{
public:
    /**
     * @brief Starts listening and accepting connections.
     *
     * Binds the socket to the configured port, lazily constructs the
     * ThreadPool sized to max_connections, and enters the accept loop.
     */
    void start();

    /**
     * @brief Signals the server to stop accepting new connections and shuts down.
     *
     * Sets the running flag to false, unblocks the accept loop, and tears
     * down the ThreadPool once in-flight connections have finished.
     */
    void stop();

    /**
     * @brief Returns the singleton Server instance, constructing it on first call.
     * @param port Port to listen on. Only used if this is the first call.
     * @param max_connections Size of the underlying ThreadPool. Only used if this is the first call.
     */
    static Server &get_instance(std::optional<std::string> port = std::nullopt, int max_connections = 10);

    /**
     * @brief Registers an observer to be notified of every accepted connection.
     * @param observer The observer to attach. Observers are never detached.
     */
    void attach(std::shared_ptr<ConnectionObserver> observer);

    /**
     * @brief Sets the policy for handling connections accepted while the
     * server is already at max_connections. If never called, accept_loop()
     * falls back to enqueuing every connection regardless of load.
     * @param strategy The strategy to use from this point on.
     */
    void set_overload_strategy(std::unique_ptr<OverloadStrategy> strategy);

    // Overload operators to implement singleton pattern
    Server(const Server &) = delete;
    Server &operator=(const Server &) = delete;
    Server(Server &&) = delete;
    Server &operator=(Server &&) = delete;

private:

    friend std::default_delete<Server>;

    /**
     * @brief Constructs a Server bound to the given port with a bounded connection pool.
     * @param port Port to listen on.
     * @param max_connections Number of worker threads in the underlying ThreadPool.
     */
    Server(std::optional<std::string> port, int max_connections);

    /**
     * @brief Closes the listening socket; owned worker resources are released automatically.
     */
    ~Server();

    /**
     * @brief Blocks on accept(), notifies observers, and admits connections according to the overload policy.
     */
    void accept_loop();

    /**
     * @brief Handles a single client connection: parses the request and serves a response.
     * @param client The accepted client connection.
     */
    void handle_connection(std::shared_ptr<Client> client);

    /**
     * @brief Serves the requested file to the client, including range requests.
     * @param client The client connection to write the response to.
     * @param request The parsed HTTP request describing the requested resource.
     */
    void serve_file(const std::shared_ptr<Client> &client, const Request &request);

    /**
     * @brief Admits a connection unconditionally: tracks it as active and
     * enqueues it onto the ThreadPool. Callers (accept_loop()'s normal
     * path, or an OverloadStrategy's admit callback) are responsible for
     * having already decided this connection should be let through.
     * @param client The connection to admit.
     */
    void admit_connection(std::shared_ptr<Client> client);

    /**
     * @brief Writes a minimal error response (status line + short text body).
     * @param client The client to write the response to.
     * @param status The status code to report.
     */
    void send_error(const std::shared_ptr<Client> &client, StatusCode status);

    /// Port the server listens on.
    std::optional<std::string> port;
    /// Listening socket file descriptor.
    int socket_fd;
    /// Maximum number of concurrent connections; sizes the ThreadPool.
    int max_connections;
    /// Flag indicating whether the accept loop should keep running.
    std::atomic<bool> running{false};
    /// Worker pool that connection-handling tasks are dispatched to, built lazily in start().
    std::unique_ptr<ThreadPool> pool;
    /// Observers notified of every accepted connection, regardless of load.
    std::vector<std::shared_ptr<ConnectionObserver>> observers;
    /// Policy for handling connections accepted while at max_connections; null means "enqueue anyway".
    std::unique_ptr<OverloadStrategy> overload_strategy;
    /// Guards active_connections and backs capacity_cv.
    std::mutex capacity_mutex;
    /// Notified whenever active_connections decreases, for BlockOverloadStrategy to wait on.
    std::condition_variable capacity_cv;
    /// Number of connections currently admitted (enqueued but not yet finished handling).
    int active_connections{0};
    /// The singleton instance.
    static std::unique_ptr<Server> instance;
};