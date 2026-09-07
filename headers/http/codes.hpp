#pragma once

// Standard library headers
#include <string_view>

/**
 * @brief HTTP status codes used by this server's responses.
 *
 * Not exhaustive — only the codes this server actually has occasion to
 * send. Extend as new response paths are added.
 */
enum class StatusCode : int
{
    Ok = 200,
    PartialContent = 206,
    NotModified = 304,
    BadRequest = 400,
    Forbidden = 403,
    NotFound = 404,
    MethodNotAllowed = 405,
    RangeNotSatisfiable = 416,
    UriTooLong = 414,
    InternalServerError = 500,
    NotImplemented = 501,
    ServiceUnavailable = 503,
    HttpVersionNotSupported = 505,
};

/**
 * @brief Returns the standard reason phrase for a status code (e.g. "Not Found").
 * @param code The status code to look up.
 */
constexpr std::string_view reason_phrase(StatusCode code)
{
    switch (code)
    {
        case StatusCode::Ok: return "OK";
        case StatusCode::PartialContent: return "Partial Content";
        case StatusCode::NotModified: return "Not Modified";
        case StatusCode::BadRequest: return "Bad Request";
        case StatusCode::Forbidden: return "Forbidden";
        case StatusCode::NotFound: return "Not Found";
        case StatusCode::MethodNotAllowed: return "Method Not Allowed";
        case StatusCode::RangeNotSatisfiable: return "Range Not Satisfiable";
        case StatusCode::UriTooLong: return "URI Too Long";
        case StatusCode::InternalServerError: return "Internal Server Error";
        case StatusCode::NotImplemented: return "Not Implemented";
        case StatusCode::ServiceUnavailable: return "Service Unavailable";
        case StatusCode::HttpVersionNotSupported: return "HTTP Version Not Supported";
    }
    return "Unknown";
}