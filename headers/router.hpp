#pragma once

// Standard library headers
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>

/**
 * @class Router
 * @brief Resolves request targets against a table of routes loaded from disk.
 *
 * Routes are simple key/value pairs read from routes.conf, one route per
 * line:
 *
 *     /path = /target
 *
 * Blank lines and lines starting with '#' are ignored. routes.conf is read
 * relative to the current working directory, the same way Server's "public"
 * root is -- the server always runs from a known working directory, so no
 * path resolution beyond that is needed.
 */
class Router
{
public:
    /**
     * @brief Loads routes.conf from the current working directory.
     * @throws std::runtime_error if routes.conf can't be opened.
     */
    Router();

    /**
     * @brief Looks up the configured target for a request path.
     * @param path The request target to resolve (e.g. "/about").
     * @return The mapped target, or std::nullopt if no route matches.
     */
    std::optional<std::string_view> resolve(std::string_view path) const;

    /// Number of routes currently loaded.
    size_t size() const;

private:
    /**
     * @brief Parses routes.conf into a lookup table.
     * @param path Filesystem path to the routes.conf file to read.
     * @return The parsed routes.
     * @throws std::runtime_error if the file can't be opened.
     */
    static std::unordered_map<std::string, std::string> load_routes(const std::string &path);

    /// Routes keyed by request path, loaded once at construction.
    std::unordered_map<std::string, std::string> routes_;
};