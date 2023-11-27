// Standard library headers
#include <csignal>
#include <thread>

// System headers
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

// Project-specific headers
#include "client.hpp"
#include "server.hpp"

int main()
{
    struct sigaction sa;
    sa.sa_handler = SIG_IGN;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGPIPE, &sa, NULL) == -1)
    {
        return 1;
    }

    server srv;

    while (true)
    {
        auto c = new client();

        socklen_t addr_len = c->getAddressLength();
        c->setSocket(accept(srv.getSocket(), reinterpret_cast<struct sockaddr *>(&c->getAddress()),
                            &addr_len));

        std::shared_ptr<client> cPtr(c);

        std::thread clientThread(&server::handleClient, &srv, cPtr);

        clientThread.detach();
    }

    return 0;
}
