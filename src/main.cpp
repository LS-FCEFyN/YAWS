// Standard library headers
#include <charconv>
#include <csignal>
#include <iostream>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>

// Project-specific headers
#include "observer/connection_logger.hpp"
#include "server.hpp"
#include "strategy/block_overload_strategy.hpp"
#include "strategy/enqueue_anyway_strategy.hpp"
#include "strategy/reject_overload_strategy.hpp"

namespace
{
void ignore_sigpipe(int)
{
}
}

int main(int argc, char *argv[])
{
    if (std::signal(SIGPIPE, ignore_sigpipe) == SIG_ERR)
    {
        std::cerr << "Error: unable to install SIGPIPE handler.\n";
        return 1;
    }

    const auto display_help = [program = std::string(argv[0])] {
        std::cout << "Usage: " << program << " [options]\n\n"
                  << "Options:\n"
                  << "  -p, --port <port>                  Port to listen on (default: 8080)\n"
                  << "  -m, --max-connections <connections> Maximum concurrent connections (default: 10)\n"
                  << "  -o, --overload-strategy <name>      block (default), reject, or unbounded\n"
                  << "  -h, --help                         Display this help message\n";
    };

    std::optional<std::string> port;
    std::optional<int> max_connections;
    std::optional<std::string> overload_strategy_name;

    for (int argument_index = 1; argument_index < argc; ++argument_index)
    {
        const std::string argument = argv[argument_index];

        if (argument == "-h" || argument == "--help")
        {
            display_help();
            return 0;
        }

        if (argument == "-p" || argument == "--port")
        {
            if (++argument_index >= argc)
            {
                std::cerr << "Error: " << argument << " requires a value.\n";
                return 1;
            }
            port = argv[argument_index];
            continue;
        }

        if (argument == "-m" || argument == "--max-connections")
        {
            if (++argument_index >= argc)
            {
                std::cerr << "Error: " << argument << " requires a value.\n";
                return 1;
            }

            const std::string value = argv[argument_index];
            int parsed_max_connections;
            const auto [end, error] = std::from_chars(
                value.data(), value.data() + value.size(), parsed_max_connections);
            if (error != std::errc{} || end != value.data() + value.size() || parsed_max_connections <= 0)
            {
                std::cerr << "Error: max connections must be a positive integer.\n";
                return 1;
            }
            max_connections = parsed_max_connections;
            continue;
        }

        if (argument == "-o" || argument == "--overload-strategy")
        {
            if (++argument_index >= argc)
            {
                std::cerr << "Error: " << argument << " requires a value.\n";
                return 1;
            }

            overload_strategy_name = argv[argument_index];
            if (overload_strategy_name != "block" && overload_strategy_name != "reject" &&
                overload_strategy_name != "unbounded")
            {
                std::cerr << "Error: --overload-strategy must be one of: block, reject, unbounded.\n";
                return 1;
            }
            continue;
        }

        std::cerr << "Error: unknown argument '" << argument << "'.\n";
        display_help();
        return 1;
    }

    try
    {
        Server *server_ptr;
        if (port && max_connections)
        {
            server_ptr = &Server::get_instance(port, *max_connections);
        }
        else if (port)
        {
            server_ptr = &Server::get_instance(port);
        }
        else if (max_connections)
        {
            server_ptr = &Server::get_instance(std::nullopt, *max_connections);
        }
        else
        {
            server_ptr = &Server::get_instance();
        }
        Server &server = *server_ptr;

        // Demonstrates the observer hook -- logs every accepted connection
        // regardless of which overload strategy ends up handling it.
        server.attach(std::make_shared<ConnectionLogger>());

        const std::string &strategy = overload_strategy_name.value_or("block");
        if (strategy == "reject")
        {
            server.set_overload_strategy(std::make_unique<RejectOverloadStrategy>());
        }
        else if (strategy == "unbounded")
        {
            server.set_overload_strategy(std::make_unique<EnqueueAnywayStrategy>());
        }
        else
        {
            server.set_overload_strategy(std::make_unique<BlockOverloadStrategy>());
        }

        server.start();
    }
    catch (const std::exception &error)
    {
        std::cerr << "Error: " << error.what() << '\n';
        return 1;
    }

    return 0;
}