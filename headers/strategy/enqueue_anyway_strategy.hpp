#pragma once

// Project-specific headers
#include "overload_strategy.hpp"

/**
 * @class EnqueueAnywayStrategy
 * @brief Admits every connection regardless of load -- the server's
 * original behavior, kept as an explicit, nameable choice rather than
 * the only option.
 */
class EnqueueAnywayStrategy : public OverloadStrategy
{
public:
    void handle_overload(std::shared_ptr<Client> client,
                          std::function<void(std::shared_ptr<Client>)> admit,
                          CapacityGate &gate) override;
};
