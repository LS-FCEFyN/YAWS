// Standard library headers
#include <fstream>
#include <stdexcept>
#include <string>

// Project-specific headers
#include "router.hpp"

namespace
{
    std::string_view trim(std::string_view s)
    {
        const auto begin = s.find_first_not_of(" \t");
        if (begin == std::string_view::npos)
            return {};
        const auto end = s.find_last_not_of(" \t");
        return s.substr(begin, end - begin + 1);
    }
}

Router::Router() : routes_(load_routes("routes.conf"))
{
}

std::unordered_map<std::string, std::string> Router::load_routes(const std::string &path)
{
    std::ifstream file(path);
    if (!file.is_open())
    {
        throw std::runtime_error("unable to open routes file: " + path);
    }

    std::unordered_map<std::string, std::string> routes;
    std::string line;
    while (std::getline(file, line))
    {
        const std::string_view trimmed_line = trim(line);
        if (trimmed_line.empty() || trimmed_line.front() == '#')
            continue;

        const auto separator = trimmed_line.find('=');
        if (separator == std::string_view::npos)
            continue;

        const std::string_view key = trim(trimmed_line.substr(0, separator));
        const std::string_view value = trim(trimmed_line.substr(separator + 1));
        if (key.empty() || value.empty())
            continue;

        routes.emplace(std::string(key), std::string(value));
    }

    return routes;
}

std::optional<std::string_view> Router::resolve(std::string_view path) const
{
    const auto it = routes_.find(std::string(path));
    return it != routes_.end() ? std::optional<std::string_view>(it->second) : std::nullopt;
}

size_t Router::size() const
{
    return routes_.size();
}