// Standard library headers
#include <sstream>

// Project-specific headers
#include "http/response.hpp"

Response::Response(StatusCode status) : status_(status) {}

void Response::set_header(std::string name, std::string value)
{
    headers_[std::move(name)] = std::move(value);
}

void Response::set_body(std::string body)
{
    set_header("Content-Length", std::to_string(body.size()));
    body_ = std::move(body);
}

std::string Response::serialize_headers() const
{
    std::ostringstream oss;
    oss << "HTTP/1.1 " << static_cast<int>(status_) << " " << reason_phrase(status_) << "\r\n";
    for (const auto &[name, value] : headers_)
    {
        oss << name << ": " << value << "\r\n";
    }
    oss << "\r\n";
    return oss.str();
}

const std::optional<std::string> &Response::body() const { return body_; }