#pragma once

// Standard library headers
#include <string_view>
#include <unordered_map>

/**
 * @brief Looks up the MIME type for a file based on its extension.
 * @param path File path or name to inspect (only the extension is used).
 * @return The matching MIME type, or "application/octet-stream" if unknown.
 */
inline std::string_view mime_type_for(std::string_view path)
{
    static const std::unordered_map<std::string_view, std::string_view> types{
        {"html", "text/html"}, {"htm", "text/html"}, {"css", "text/css"},
        {"js", "application/javascript"}, {"json", "application/json"},
        {"txt", "text/plain"}, {"png", "image/png"}, {"jpg", "image/jpeg"},
        {"jpeg", "image/jpeg"}, {"gif", "image/gif"}, {"svg", "image/svg+xml"},
        {"ico", "image/x-icon"}, {"pdf", "application/pdf"},
    };

    const auto dot = path.find_last_of('.');
    if (dot == std::string_view::npos)
    {
        return "application/octet-stream";
    }

    const auto it = types.find(path.substr(dot + 1));
    return it != types.end() ? it->second : "application/octet-stream";
}