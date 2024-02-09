#pragma once

// Standard library headers
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>

// Project-specific headers
#include "client.hpp"
#include "router.hpp"

/**
 * @class server
 * @brief Represents a web server that handles incoming client requests.
 *
 * This class is responsible for setting up a server socket, accepting client connections,
 * and serving HTTP requests to clients. It uses a router to manage routes and a mapping of
 * MIME types for different file extensions.
 */
class server
{
private:
    /// Socket file descriptor for the server.
    int sockfd;
    /// Router for managing server routes.
    router srvRouter;
    /// Mapping of file extensions to MIME types.
    std::unordered_map<std::string, std::string> mimeMap;

public:
    /**
     * @brief Constructs a server instance with an optional port number.
     * @param port Optional port number as a string. If not provided, the server will choose a default port.
     */
    server(std::optional<std::string> port = std::nullopt);

    /**
     * @brief Destructor for the server class.
     *
     * Ensures that the server socket is properly closed and released.
     */
    ~server();

    /**
     * @brief Returns the socket file descriptor of the server.
     * @return The socket file descriptor.
     */
    int getSocket();

    /**
     * @brief Handles a client connection by processing the client's requests.
     * @param client Shared pointer to a client instance representing the connected client.
     */
    void handleClient(std::shared_ptr<client> client);

    /**
     * @brief Serves a specific request to a client based on the requested route.
     * @param client Shared pointer to a client instance representing the connected client.
     * @param route Optional reference to a constant string representing the route to serve.
     *              If not provided, the server will determine the route based on the client's request.
     */
    void serveRequest(std::shared_ptr<client> client,
                      std::optional<std::reference_wrapper<const std::string>> route = std::nullopt);

    /**
     * @brief Sends an HTTP error response to a client.
     * @param client Shared pointer to a client instance representing the connected client.
     * @param httpStatus HTTP status code as a string.
     * @param httpMessage HTTP status message as a string.
     */
    void sendError(std::shared_ptr<client> client,
                   const std::string &httpStatus, const std::string &httpMessage);

    /**
     * @brief Determines the MIME type for a given resource based on its file extension.
     * @param resource Resource name with its file extension.
     * @return The MIME type corresponding to the resource's file extension.
     */
    std::string mime(const std::string &resource);
};
