#pragma once

// Project-specific headers
#include "overload_strategy.hpp"

/**
 * @class BlockOverloadStrategy
 * @brief Holds a connection until a slot frees up, then admits it.
 *
 * accept_loop() calls this synchronously, so blocking here also pauses
 * accept() from pulling in further connections until capacity returns --
 * the same backpressure a bounded, blocking system gets for free.
 */
class BlockOverloadStrategy : public OverloadStrategy
{
public:
    void handle_overload(std::shared_ptr<Client> client,
                          std::function<void(std::shared_ptr<Client>)> admit,
                          CapacityGate &gate) override;
};
