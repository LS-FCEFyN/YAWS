#pragma once

class Client;

/**
 * @class ConnectionObserver
 * @brief Notified when the Server accepts a new connection.
 *
 * Observers are for side effects only (logging, metrics, and the like) --
 * they can't influence whether the connection is admitted. That decision
 * belongs to OverloadStrategy, not to whoever happens to be listening here.
 */
class ConnectionObserver
{
public:
    virtual ~ConnectionObserver() = default;

    /**
     * @brief Called right after a connection is accepted, before the
     * server has decided whether it will be admitted or turned away.
     * @param client The connection that was just accepted.
     */
    virtual void on_connection_accepted(const Client &client) = 0;
};
