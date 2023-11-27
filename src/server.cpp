// Standard library headers
#include <filesystem>
#include <fstream>
#include <iostream>
#include <ranges>
#include <sstream>
#include <stdexcept>
#include <vector>

// Project-specific headers
#include "server.hpp"

// System headers
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>

server::server(std::optional<std::string> port) : srvRouter("routes.conf") {
  struct addrinfo hints = {}, *bind_address;

  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  if (port.has_value()) {
    if (getaddrinfo(0, port.value().c_str(), &hints, &bind_address) != 0) {
      throw std::runtime_error("getaddrinfo failed");
    }
  } else {
    if (getaddrinfo(0, "80", &hints, &bind_address) != 0) {
      throw std::runtime_error("getaddrinfo with default port failed");
    }
  }

  this->sockfd = socket(bind_address->ai_family, bind_address->ai_socktype,
                        bind_address->ai_protocol);

  if (this->sockfd == 0) {
    throw std::runtime_error("socket failed");
  }

  int wizzardy = 1;

  if (setsockopt(this->sockfd, SOL_SOCKET, SO_REUSEADDR, &wizzardy,
                 sizeof(int)) < 0) {
    throw std::runtime_error("setsockopt(SO_REUSEADDR)");
  }

#ifdef SO_REUSEPORT
  if (setsockopt(this->sockfd, SOL_SOCKET, SO_REUSEPORT, &wizzardy,
                 sizeof(int)) < 0) {
    throw std::runtime_error("setsockopt(SO_REUSEPORT)");
  }
#endif

  if (bind(this->sockfd, bind_address->ai_addr, bind_address->ai_addrlen)) {
    throw std::runtime_error("bind() failed");
  }

  freeaddrinfo(bind_address);

  /* Populate mimeMap before we start listening for incomming connections */
  if (std::ifstream file("mime"); file.is_open()) {
    std::string line;
    while (std::getline(file, line)) {
      if (line.starts_with('#')) {
        continue;
      }

      std::stringstream ss(line);
      std::string key, value;
      if (std::getline(ss, key, ' ') && std::getline(ss, value)) {
        mimeMap.insert(std::make_pair(key, value));
      }
    }
  } else {
    throw std::runtime_error("Unable to open file");
  }

  if (listen(this->sockfd, SOMAXCONN) < 0) {
    throw std::runtime_error("listen() failed");
  }
}

int server::getSocket() { return this->sockfd; }

void server::sendError(std::shared_ptr<client> client,
                       const std::string &httpStatus,
                       const std::string &httpMessage) {
  std::ostringstream oss;
  
  oss << "HTTP/1.1 " << httpStatus << " " << httpMessage << "\r\n"
        << "Connection: close\r\n";

  std::string response = oss.str();

  send(client->getSocket(), response.c_str(), response.length(), 0);
}

std::string server::mime(const std::string &resource) {
  std::string extension = resource.substr(resource.find_last_of(".") + 1);
  if (mimeMap.contains(extension)) {
    return mimeMap.find(extension)->second;
  }
  return "application/octet-stream";
}

void server::serveRequest(
    std::shared_ptr<client> client,
    std::optional<std::reference_wrapper<const std::string>> route) {
  std::string path = route.has_value() ? std::string(route->get())
                                       : client->getRequest().at("path");

  std::ostringstream oss;

  oss << "public" << path;

  std::string absolutePath = oss.str();

  std::ifstream file(absolutePath, std::ios::in);
  if (!file.is_open() || std::filesystem::is_directory(absolutePath)) {
    sendError(client, "400", "Bad Request");
    return;
  }

  if (client->getRequest().contains("Range")) {
    long start, end = -1;

    std::string rangeHeader = client->getRequest().at("Range");

    std::istringstream iss(rangeHeader.substr(rangeHeader.find('=') + 1));
    char dash;
    iss >> start >> dash;
    if (dash == '-')
      iss >> end;

    file.seekg(0, std::ios::end);
    long fileSize = file.tellg();

    if (start < 0 || (end >= 0 && end < start) ||
        (end >= fileSize && start != fileSize)) {
      sendError(client, "400", "Bad Request");
      return;
    }

    if (start >= fileSize) {
      sendError(client, "416", "Requested Range Not Satisfiable");
      return;
    }

    if (end == -1 || end >= fileSize) {
      end = fileSize - 1;
    }

    file.seekg(start, std::ios::beg);

    size_t cl = end - start + 1;

    std::stringstream response;

    response << "HTTP/1.1 206 Partial Content\r\n"
             << "Connection: close\r\n"
             << "Content-Length: " << cl << "\r\n"
             << "Content-Type: " << mime(path) << "\r\n"
             << "Content-Range: bytes " << start << "-" << end << "/"
             << fileSize << "\r\n\r\n";

    std::string responseStr = response.str();
    send(client->getSocket(), responseStr.c_str(), responseStr.size(), 0);

    std::vector<char> buf(1024);
    size_t bytes_sent = 0;

    while (bytes_sent < cl) {
      file.read(buf.data(), buf.size());
      send(client->getSocket(), buf.data(), file.gcount(), 0);
      bytes_sent += file.gcount();
    }
  } else {
    file.seekg(0, std::ios::end);
    size_t fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    std::stringstream response;

    response << "HTTP/1.1 200 OK\r\n"
             << "Connection: close\r\n"
             << "Content-Length: " << fileSize << "\r\n"
             << "Content-Type: " << mime(path) << "\r\n\r\n";

    std::string responseStr = response.str();
    send(client->getSocket(), responseStr.c_str(), responseStr.size(), 0);

    std::vector<char> buf(fileSize);

    if (file.read(&buf[0], fileSize)) {
      send(client->getSocket(), &buf[0], fileSize, 0);
    }
  }

  file.close();
}

void server::handleClient(std::shared_ptr<client> client) {
  if (client->receiveParse()) {
    if (!(client->getRequest().at("method") == "GET")) {
      sendError(client, "501", "Not Implemented");
    } else {
      if (srvRouter.contains(client->getRequest().at("path"))) {
        serveRequest(client, srvRouter.find(client->getRequest().at("path")));
      } else {
        serveRequest(client);
      }
    }
  }
}

server::~server() { close(this->sockfd); }
