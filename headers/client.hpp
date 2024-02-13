#pragma once

// Standard library headers
#include <string>
#include <unordered_map>

// System headers
#include <sys/socket.h>

/**
 * @class client
 * @brief Represents a network client capable of sending and receiving data over sockets.
 *
 * This class encapsulates the details of socket communication, providing an interface for
 * sending requests and parsing responses. It manages the underlying socket file descriptor
 * and address information.
 */
class client
{
private:
    /// Length of the address structure.
    socklen_t addressLength;
    /// Storage for the socket address.
    struct sockaddr_storage address;
    /// File descriptor for the socket connection.
    int sockfd;
    /// Request parameters stored as key-value pairs.
    std::unordered_map<std::string, std::string> request;

public:
    /**
     * @brief Default constructor for the client class.
     *
     * Initializes the address length and prepares the client for receiving data.
     */
    client();

    /**
     * @brief Destructor for the client class.
     *
     * Closes the socket connection if open and performs any necessary cleanup.
     */
    ~client();

    /**
     * @brief Getter for the socket address structure.
     * @return Reference to the socket address structure.
     */
    struct sockaddr_storage &getAddress();

    /**
     * @brief Getter for the length of the socket address structure.
     * @return The length of the socket address structure.
     */
    socklen_t getAddressLength();

    /**
     * @brief Getter for the socket file descriptor.
     * @return The socket file descriptor.
     */
    int getSocket();

    /**
     * @brief Getter for the current request parameters.
     * @return The request parameters as a key-value map.
     */
    std::unordered_map<std::string, std::string> getRequest();

    /**
     * @brief Setter for the socket file descriptor.
     * @param sockfd The new socket file descriptor to assign.
     */
    void setSocket(int sockfd);

    /**
     * @brief Receives data from the socket and parses it into request parameters.
     * @return True if the request is successfully parsed, false otherwise.
     *
     * This method receives data from the socket and attempts to parse it into the request parameters.
     * It dynamically resizes the temporary buffer as needed and ensures the entire header is captured.
     */
    bool receiveParse();
};
