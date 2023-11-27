// Standard library headers
#include <fstream>
#include <sstream>

// Project-specific headers
#include "router.hpp"


router::router(const std::string &configFile)
{
    std::ifstream file(configFile);
    if (!file.is_open())
    {
        routes["/"] = "/index.html";
    }
    else
    {
        std::string line;
        while (std::getline(file, line))
        {
            std::istringstream iss(line);
            std::string handler, path;
            if (iss >> handler >> path)
            {
                routes[handler] = path;
            }
        }
    }
}

bool router::contains(const std::string &key)
{
    return routes.contains(key);
}

const std::string &router::find(const std::string &key)
{
    return routes.find(key)->second;
}

router::~router()
{
}