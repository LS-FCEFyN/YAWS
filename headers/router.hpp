#pragma once

// Standard library headers
#include <string>
#include <unordered_map>

class router
{
private:
    std::unordered_map<std::string, std::string> routes;

public:
    router(const std::string &configFile);
    ~router();

    bool contains(const std::string &key);
    const std::string &find(const std::string &key);
};