#pragma once

// Standard library headers
#include <optional>
#include <string_view>

/**
 * @brief HTTP methods this server accepts.
 *
 * As a static file server, only retrieval methods are supported. Anything
 * else in a request line should be treated as unsupported (405/501),
 * not extended here casually.
 */
enum class Method
{
    Get,
    Head,
};

/**
 * @brief Parses a request-line method token into a Method.
 * @param token The method token as it appears in the request (e.g. "GET").
 * @return The corresponding Method, or std::nullopt if not supported.
 */
constexpr std::optional<Method> parse_method(std::string_view token)
{
    if (token == "GET")
        return Method::Get;
    if (token == "HEAD")
        return Method::Head;
    return std::nullopt;
}

/**
 * @brief Returns the canonical string representation of a Method.
 * @param method The method to convert.
 */
constexpr std::string_view to_string(Method method)
{
    switch (method)
    {
    case Method::Get:
        return "GET";
    case Method::Head:
        return "HEAD";
    }
    return "UNKNOWN";
}