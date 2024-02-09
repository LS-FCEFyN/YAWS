#pragma once

// Standard library headers
#include <string>
#include <unordered_map>

/**
 * @class router
 * @brief Manages routing configurations based on string identifiers.
 *
 * This class encapsulates the logic for handling and querying route configurations.
 * It uses an unordered_map to store and retrieve routes efficiently.
 */
class router
{
private:
    /// Map of routes where the key is a string identifier and the value is the route configuration.
    std::unordered_map<std::string, std::string> routes;

public:
    /**
     * @brief Constructor for the router class.
     * @param configFile Path to the configuration file for setting up the routes.
     *
     * This constructor reads the configuration file and populates the internal routes map.
     * If the file cannot be opened, it sets a default route for the root path ("/") to "/index.html".
     */
    router(const std::string &configFile);

    /**
     * @brief Destructor for the router class.
     *
     * Since the router class does not allocate any dynamic memory, the destructor
     * is straightforward and does not need to perform any cleanup actions.
     */
    ~router();

    /**
     * @brief Checks if a given key exists in the routes map.
     * @param key String identifier for the route to check.
     * @return True if the key exists in the routes map, false otherwise.
     */
    bool contains(const std::string &key);

    /**
     * @brief Finds and retrieves the value associated with a given key in the routes map.
     * @param key String identifier for the route to find.
     * @return Reference to the value in the routes map corresponding to the key.
     * @throws std::out_of_range if the key is not found in the map.
     */
    const std::string &find(const std::string &key);
};
