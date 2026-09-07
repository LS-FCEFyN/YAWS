// Project-specific headers
#include "client.hpp"
#include "http/codes.hpp"
#include "http/response.hpp"

#include "strategy/reject_overload_strategy.hpp"

// System headers
#include <cerrno>
#include <unistd.h>

namespace
{
bool write_all(int socket_fd, const char *data, size_t size)
{
    while (size > 0)
    {
        const ssize_t bytes_written = write(socket_fd, data, size);
        if (bytes_written < 0)
        {
            if (errno == EINTR)
                continue;
            return false;
        }
        if (bytes_written == 0)
            return false;
        data += bytes_written;
        size -= static_cast<size_t>(bytes_written);
    }
    return true;
}
}

void RejectOverloadStrategy::handle_overload(std::shared_ptr<Client> client,
                                              std::function<void(std::shared_ptr<Client>)>,
                                              CapacityGate &)
{
    // `admit` and `gate` are deliberately unused: this strategy never lets
    // the connection through. Closing happens on its own once `client`
    // goes out of scope, via Client's RAII destructor.
    Response response(StatusCode::ServiceUnavailable);
    response.set_header("Connection", "close");
    response.set_header("Retry-After", "1");
    response.set_header("Content-Type", "text/plain");
    response.set_body(std::string(reason_phrase(StatusCode::ServiceUnavailable)));

    const std::string headers = response.serialize_headers();
    if (!write_all(client->get_socket_fd(), headers.data(), headers.size()))
        return;
    if (response.body())
    {
        write_all(client->get_socket_fd(), response.body()->data(), response.body()->size());
    }
}
