// Project-specific headers
#include "strategy/misc/server_capacity_gate.hpp"

ServerCapacityGate::ServerCapacityGate(std::mutex &mutex, std::condition_variable &cv, int &active, int max)
    : mutex_(mutex), cv_(cv), active_(active), max_(max)
{
}

int ServerCapacityGate::active_connections() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return active_;
}

int ServerCapacityGate::max_connections() const { return max_; }

void ServerCapacityGate::wait_for_capacity()
{
    std::unique_lock<std::mutex> lock(mutex_);
    cv_.wait(lock, [this] { return active_ < max_; });
}
