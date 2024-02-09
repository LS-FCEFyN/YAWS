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
    std::vector<char> buffer(4096);
    ssize_t received = recv(sockfd, buffer.data(), buffer.size(), 0);

    while (received > static_cast<ssize_t>(buffer.size()))
    {
        buffer.resize(buffer.size() * 2);
        received += recv(sockfd, buffer.data() + received, buffer.size() - received, 0);
    }

    std::string tmpStr(buffer.begin(), buffer.end());
    tmpStr.push_back('\0');

    std::size_t headerEnd = tmpStr.find("\r\n\r\n");
    if (headerEnd == std::string::npos)
    {
        return false;
    }

    std::istringstream requestLineStream(tmpStr.substr(0, headerEnd));
    std::string method, path, version;
    if (!(requestLineStream >> method >> path >> version))
    {
        return false;
    }

    request["method"] = method;
    request["path"] = path;
    request["version"] = version;

    std::size_t headerLine = tmpStr.find("\r\n");

    while (headerLine != std::string::npos && headerLine < headerEnd)
    {
        std::size_t nextHeaderLine = tmpStr.find("\r\n", headerLine + 2);
        std::istringstream headerLineStream(tmpStr.substr(headerLine + 2, nextHeaderLine - headerLine - 2));
        std::string header, value;
        if (!(headerLineStream >> header >> value))
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