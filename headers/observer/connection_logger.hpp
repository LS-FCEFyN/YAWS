#pragma once

// Standard library headers
#include <iostream>

// Project-specific headers
#include "client.hpp"
#include "connection_observer.hpp"

/**
 * @class ConnectionLogger
 * @brief Example ConnectionObserver that logs each accepted connection's
 * socket descriptor to stdout.
 */
class ConnectionLogger : public ConnectionObserver
{
public:
    void on_connection_accepted(const Client &client) override
    {
        std::cout << "[connection] accepted fd=" << client.get_socket_fd() << '\n';
    }
};
