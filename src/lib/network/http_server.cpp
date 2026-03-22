#include "http_server.hpp"

#include <arpa/inet.h>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/tcp.h>

namespace network
{
    HttpServer::HttpServer()
        : port_(8080), serverSocket_(-1), running_(false), handler_(nullptr)
    {
    }

    HttpServer::~HttpServer()
    {
        HttpServer::stop();
    }

    void HttpServer::setPort(int port)
    {
        port_ = port;
    }

    void HttpServer::setRequestHandler(RequestHandler handler)
    {
        handler_ = handler;
    }

    void HttpServer::start()
    {
        if (!handler_) {
            throw std::runtime_error("handler is unset");
        }

        serverSocket_ = socket(AF_INET, SOCK_STREAM, 0);
        if (serverSocket_ < 0)
        {
            throw std::runtime_error("error creating socket");
        }

        auto enableSockOpt = [this](int level, int optname) {
            int opt = 1;
            if (setsockopt(serverSocket_, level, optname, &opt, sizeof(opt)) < 0)
            {
                close(serverSocket_);
                serverSocket_ = -1;
                throw std::runtime_error("error setting socket options");
            }
        };
        enableSockOpt(SOL_SOCKET, SO_REUSEADDR);
        enableSockOpt(IPPROTO_TCP, TCP_NODELAY);
        enableSockOpt(IPPROTO_TCP, TCP_FASTOPEN);

        sockaddr_in serverAddr{};
        memset(&serverAddr, 0, sizeof(serverAddr));
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
        serverAddr.sin_port = htons(static_cast<uint16_t>(port_));

        if (bind(serverSocket_, reinterpret_cast<struct sockaddr *>(&serverAddr), sizeof(serverAddr)) < 0)
        {
            close(serverSocket_);
            serverSocket_ = -1;
            throw std::runtime_error("error binding socket to port");
        }

        if (listen(serverSocket_, SOMAXCONN) < 0)
        {
            close(serverSocket_);
            serverSocket_ = -1;
            throw std::runtime_error("error listening on socket");
        }

        running_ = true;
        std::cout << "HTTP Server listening on port " << port_ << std::endl;
    }

    void HttpServer::stop()
    {
        if (serverSocket_ >= 0)
        {
            close(serverSocket_);
            serverSocket_ = -1;
        }
        running_ = false;
    }

    bool HttpServer::isRunning() const
    {
        return running_;
    }

    void HttpServer::parseHttpRequest(const std::string &rawRequest, std::string &path, std::string &method, std::string &body)
    {
        // Find the separator between headers and body
        size_t bodyStart = rawRequest.find("\r\n\r\n");
        std::string headerSection = rawRequest;
        body = "";

        if (bodyStart != std::string::npos)
        {
            headerSection = rawRequest.substr(0, bodyStart);
            bodyStart += 4;  // Skip past "\r\n\r\n"
            if (bodyStart < rawRequest.length())
            {
                body = rawRequest.substr(bodyStart);
            }
        }

        std::istringstream stream(headerSection);
        std::string firstLine;
        std::getline(stream, firstLine);

        // METHOD PATH HTTP/VERSION
        std::istringstream lineStream(firstLine);
        path = "/";

        lineStream >> method >> path;

        // ignore query parameters
        size_t queryPos = path.find('?');
        if (queryPos != std::string::npos)
        {
            path = path.substr(0, queryPos);
        }

        if (path.empty())
        {
            path = "/";
        }
    }

    std::string HttpServer::buildHttpResponse(const std::string &statusCode, const std::string &contentType,
                                              const std::string &body)
    {
        std::ostringstream response;
        response << "HTTP/1.1 " << statusCode << "\r\n";
        response << "Content-Type: " << contentType << "\r\n";
        response << "Content-Length: " << body.length() << "\r\n";
        response << "Connection: close\r\n";
        response << "\r\n";
        response << body;

        return response.str();
    }

    bool HttpServer::acceptConnection()
    {
        if (!running_ || serverSocket_ < 0)
        {
            return false;
        }

        sockaddr_in clientAddr{};
        socklen_t clientAddrLen = sizeof(clientAddr);

        int clientSocket = accept(serverSocket_, reinterpret_cast<struct sockaddr *>(&clientAddr), &clientAddrLen);
        if (clientSocket < 0)
        {
            std::cerr << "Error accepting connection" << std::endl;
            return false;
        }

        char buffer[4096] = {0};
        ssize_t bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);

        if (bytesReceived > 0)
        {
            buffer[bytesReceived] = '\0';
            std::string rawRequest(buffer);

            std::string method;
            std::string path;
            std::string body;
            parseHttpRequest(rawRequest, path, method, body);

            HttpResponse response = handler_(path, method, body);
            std::string responseBody = response.content;
            std::string contentType = response.type;

            if (contentType.empty()) {
                if (path.find(".html") != std::string::npos)
                {
                    contentType = "text/html";
                }
                else if (path.find(".css") != std::string::npos)
                {
                    contentType = "text/css";
                }
                else if (path.find(".js") != std::string::npos)
                {
                    contentType = "application/javascript";
                }
            }

            // Build and send response
            std::string httpResponse;
            if (response.code == 404)
            {
                httpResponse = buildHttpResponse("404 Not Found", "text/html", "<h1>404 Not Found</h1>");
            }
            else if (response.code == 401)
            {
                httpResponse = buildHttpResponse("401 Unauthorized", "", "");
            } else
            {
                httpResponse = buildHttpResponse("200 OK", contentType, responseBody);
            }

            send(clientSocket, httpResponse.c_str(), httpResponse.length(), 0);
        }

        close(clientSocket);
        return true;
    }

}  // namespace network
