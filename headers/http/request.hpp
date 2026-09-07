#pragma once

// Standard library headers
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>

// Project-specific headers
#include "methods.hpp"

/**
 * @class Request
 * @brief Represents a parsed HTTP request received from a Client.
 */
class Request
{
public:
    /**
     * @brief Parses a raw HTTP request into a Request.
     * @param raw The raw bytes read from the client socket.
     * @return The parsed Request, or std::nullopt if it's malformed or unsupported.
     */
    static std::optional<Request> parse(std::string_view raw);

    /// The request's HTTP method.
    std::optional<Method> method() const;

    /// The requested target (path), as sent in the request line.
    const std::string &target() const;

    /// The HTTP version string as sent in the request line (e.g. "HTTP/1.1").
    const std::string &version() const;

    /**
     * @brief Looks up a header value by name.
     * @param name Header name (matched case-sensitively for now).
     * @return The header's value, or std::nullopt if not present.
     */
    std::optional<std::string_view> header(std::string_view name) const;

private:
    Request(std::optional<Method> method, std::string target, std::string version,
            std::unordered_map<std::string, std::string> headers);

    std::optional<Method> method_;
    std::string target_;
    std::string version_;
    std::unordered_map<std::string, std::string> headers_;
};