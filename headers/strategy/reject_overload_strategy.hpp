#pragma once

// Project-specific headers
#include "overload_strategy.hpp"

/**
 * @class RejectOverloadStrategy
 * @brief Refuses a connection outright once the server is at capacity,
 * responding with 503 Service Unavailable before closing the socket.
 */
class RejectOverloadStrategy : public OverloadStrategy
{
public:
    void handle_overload(std::shared_ptr<Client> client,
                          std::function<void(std::shared_ptr<Client>)> admit,
                          CapacityGate &gate) override;
};
