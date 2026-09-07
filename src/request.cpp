#include <sstream>
#include <vector>

// Project-specific headers
#include "http/request.hpp"

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

std::optional<Request> Request::parse(std::string_view raw)
{
    std::vector<std::string_view> lines;
    size_t pos = 0;
    while (pos <= raw.size())
    {
        const auto eol = raw.find('\n', pos);
        std::string_view line = (eol == std::string_view::npos) ? raw.substr(pos) : raw.substr(pos, eol - pos);
        if (!line.empty() && line.back() == '\r')
            line.remove_suffix(1);
        lines.push_back(line);
        if (eol == std::string_view::npos || line.empty())
            break;
        pos = eol + 1;
    }

    if (lines.empty() || lines.front().empty())
        return std::nullopt;

    std::istringstream request_line{std::string(lines.front())};
    std::string method_token, target, version;
    if (!(request_line >> method_token >> target >> version))
        return std::nullopt;

    std::unordered_map<std::string, std::string> headers;
    for (size_t i = 1; i < lines.size(); ++i)
    {
        if (lines[i].empty())
            continue;
        const auto colon = lines[i].find(':');
        if (colon == std::string_view::npos)
            continue;
        headers.emplace(std::string(trim(lines[i].substr(0, colon))), std::string(trim(lines[i].substr(colon + 1))));
    }

    return Request(parse_method(method_token), std::move(target), std::move(version), std::move(headers));
}

Request::Request(std::optional<Method> method, std::string target, std::string version,
                 std::unordered_map<std::string, std::string> headers)
    : method_(method), target_(std::move(target)), version_(std::move(version)), headers_(std::move(headers))
{
}

std::optional<Method> Request::method() const { return method_; }
const std::string &Request::target() const { return target_; }
const std::string &Request::version() const { return version_; }

std::optional<std::string_view> Request::header(std::string_view name) const
{
    const auto it = headers_.find(std::string(name));
    return it != headers_.end() ? std::optional<std::string_view>(it->second) : std::nullopt;
}