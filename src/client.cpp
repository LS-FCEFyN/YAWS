// Standard library headers
#include <algorithm>
#include <sstream>
#include <vector>

// Project-specific headers
#include "client.hpp"

// System headers
#include <netdb.h>
#include <sys/types.h>
#include <unistd.h>

client::client()
{
    this->addressLength = sizeof(this->address);
}

bool client::receiveParse()
{
    std::vector<char> tmp(4096);
    ssize_t received = recv(sockfd, &tmp[0],
                            tmp.size(), 0);

    if (received > static_cast<ssize_t>(tmp.size()))
    {
        size_t remaining = tmp.size() - received;
        tmp.resize(tmp.size() + remaining);
        received = recv(sockfd, &tmp[0] + received, remaining, 0);
    }
    else if (received <= 0)
    {
        return false;
    }

    tmp.push_back('\0');

    const std::string tmpStr(tmp.begin(), tmp.end());

    std::size_t headerEnd = tmpStr.find("\r\n\r\n");
    if (headerEnd == std::string::npos)
    {
        return false;
    }

    std::string method, path, version;
    std::istringstream requestLine(tmpStr.substr(0, headerEnd));
    if (!(requestLine >> method >> path >> version))
    {
        return false;
    }

    request["method"] = method;
    request["path"] = path;
    request["version"] = version;

    std::size_t headerLine = tmpStr.find("\r\n");

    while (headerLine < headerEnd)
    {
        std::size_t nextHeaderLine = tmpStr.find("\r\n", headerLine + 2);
        std::istringstream requestLine(tmpStr.substr(headerLine + 2, nextHeaderLine - headerLine - 2));
        std::string header, value;
        if (!(requestLine >> header >> value))
        {
            return false;
        }

        header.erase(std::remove(header.begin(), header.end(), ':'), header.end());

        request[header] = value;

        headerLine = nextHeaderLine;
    }

    return true;
}

struct sockaddr_storage &client::getAddress()
{
    return this->address;
}

int client::getSocket()
{
    return this->sockfd;
}

socklen_t client::getAddressLength()
{
    return this->addressLength;
}

void client::setSocket(int sockfd)
{
    this->sockfd = sockfd;
}

std::unordered_map<std::string, std::string> client::getRequest()
{
    return this->request;
}

client::~client()
{
    close(this->sockfd);
}