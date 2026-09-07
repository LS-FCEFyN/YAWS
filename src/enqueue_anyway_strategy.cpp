// Project-specific headers
#include "strategy/enqueue_anyway_strategy.hpp"

void EnqueueAnywayStrategy::handle_overload(std::shared_ptr<Client> client,
                                             std::function<void(std::shared_ptr<Client>)> admit,
                                             CapacityGate &)
{
    admit(std::move(client));
}
