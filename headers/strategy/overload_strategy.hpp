#pragma once

// Standard library headers
#include <functional>
#include <memory>

// Project-specific headers
#include "misc/capacity_gate.hpp"

class Client;

/**
 * @class OverloadStrategy
 * @brief Decides what happens to a connection accepted while the server
 * is already handling max_connections other connections.
 */
class OverloadStrategy
{
public:
    virtual ~OverloadStrategy() = default;

    /**
     * @brief Handles a connection that arrived while the server is at capacity.
     * @param client The connection that just came in.
     * @param admit Callback that enqueues the connection for normal
     * processing -- call it if this strategy decides to let it through.
     * @param gate Lets the strategy check current load or block until a
     * slot frees up, without depending on Server directly.
     */
    virtual void handle_overload(std::shared_ptr<Client> client,
                                  std::function<void(std::shared_ptr<Client>)> admit,
                                  CapacityGate &gate) = 0;
};
