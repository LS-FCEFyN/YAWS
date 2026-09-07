// Project-specific headers
#include "strategy/block_overload_strategy.hpp"

void BlockOverloadStrategy::handle_overload(std::shared_ptr<Client> client,
                                             std::function<void(std::shared_ptr<Client>)> admit,
                                             CapacityGate &gate)
{
    gate.wait_for_capacity();
    admit(std::move(client));
}
