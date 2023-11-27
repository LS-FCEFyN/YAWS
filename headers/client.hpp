#pragma once

// Standard library headers
#include <string>
#include <unordered_map>

// System headers
#include <sys/socket.h>

class client
{
private:
    socklen_t addressLength;
    struct sockaddr_storage address;
    char addressBuffer[128];
    int sockfd;
    std::unordered_map<std::string, std::string> request;

public:
    client();
    ~client();

    struct sockaddr_storage &getAddress();
    socklen_t getAddressLength();
    int getSocket();
    std::unordered_map<std::string, std::string> getRequest();
    void setSocket(int sockfd);
    bool receiveParse();
};