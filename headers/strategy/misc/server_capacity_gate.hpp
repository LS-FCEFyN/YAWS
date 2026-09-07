#pragma once

// Standard library headers
#include <condition_variable>
#include <mutex>

// Project-specific headers
#include "capacity_gate.hpp"

/**
 * @class ServerCapacityGate
 * @brief Concrete CapacityGate backed by a mutex/condition_variable pair
 * owned by Server, constructed on demand so OverloadStrategy implementations
 * never need to see Server itself.
 */
class ServerCapacityGate : public CapacityGate
{
public:
    /**
     * @brief Wraps the given mutex/condition_variable/counter, none of which
     * are owned by this object -- Server owns all of them for its lifetime.
     * @param mutex Guards active.
     * @param cv Notified whenever active decreases.
     * @param active The current number of in-flight connections.
     * @param max The connection ceiling.
     */
    ServerCapacityGate(std::mutex &mutex, std::condition_variable &cv, int &active, int max);

    int active_connections() const override;
    int max_connections() const override;
    void wait_for_capacity() override;

private:
    std::mutex &mutex_;
    std::condition_variable &cv_;
    int &active_;
    int max_;
};
