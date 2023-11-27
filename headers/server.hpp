#pragma once

// Standard library headers
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>

// Project-specific headers
#include "client.hpp"
#include "router.hpp"

class server
{
private:
    int sockfd;
    router srvRouter;
    std::unordered_map<std::string, std::string> mimeMap;

public:
    server(std::optional<std::string> port = std::nullopt);
    ~server();
    
    int getSocket();
    void handleClient(std::shared_ptr<client> client);
    void serveRequest(std::shared_ptr<client> client,
                      std::optional<std::reference_wrapper<const std::string>> route = std::nullopt);
    void sendError(std::shared_ptr<client> client,
                   const std::string &httpStatus, const std::string &httpMessage);
    std::string mime(const std::string &resource);
};