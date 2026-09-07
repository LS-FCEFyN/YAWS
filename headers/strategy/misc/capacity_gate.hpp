#pragma once

/**
 * @class CapacityGate
 * @brief Read/wait access to the Server's current connection load, without
 * exposing Server itself to OverloadStrategy implementations.
 */
class CapacityGate
{
public:
    virtual ~CapacityGate() = default;

    /**
     * @brief The number of connections currently being handled.
     */
    virtual int active_connections() const = 0;

    /**
     * @brief The configured connection ceiling.
     */
    virtual int max_connections() const = 0;

    /**
     * @brief Blocks the calling thread until active_connections() drops
     * below max_connections().
     */
    virtual void wait_for_capacity() = 0;
};
