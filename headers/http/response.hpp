#pragma once

// Standard library headers
#include <optional>
#include <string>
#include <unordered_map>

// Project-specific headers
#include "codes.hpp"

/**
 * @class Response
 * @brief Skeleton for an HTTP response: status line, headers, and an
 * optional in-memory body for generated (non-file) responses.
 *
 * The body here is only for responses the server generates itself, like
 * error pages — the static-file path writes headers via this class, then
 * streams the file body separately with sendfile().
 */
class Response
{
public:
    /**
     * @brief Constructs a Response with the given status.
     * @param status The HTTP status code for this response.
     */
    explicit Response(StatusCode status);

    /**
     * @brief Sets (or overwrites) a header.
     * @param name Header name.
     * @param value Header value.
     */
    void set_header(std::string name, std::string value);

    /**
     * @brief Sets the in-memory body and updates Content-Length accordingly.
     * @param body The response body.
     */
    void set_body(std::string body);

    /**
     * @brief Serializes the status line and headers, terminated by a blank
     * line, ready to write() to the client socket ahead of any body.
     */
    std::string serialize_headers() const;

    /// The in-memory body, if this response carries one (contrast: file responses use sendfile()).
    const std::optional<std::string> &body() const;

private:
    StatusCode status_;
    std::unordered_map<std::string, std::string> headers_;
    std::optional<std::string> body_;
};