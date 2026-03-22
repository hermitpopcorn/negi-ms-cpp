#pragma once

#include "lib/network.hpp"

namespace network
{
    class HttpServer : public HttpServerInterface
    {
      public:
        HttpServer();
        ~HttpServer();

        void setPort(int port) override;
        void setRequestHandler(RequestHandler handler) override;

        void start() override;
        void stop() override;
        bool isRunning() const override;
        bool acceptConnection() override;

      private:
        int port_;
        int serverSocket_;
        bool running_;
        RequestHandler handler_;

        void parseHttpRequest(const std::string &rawRequest, std::string &path, std::string &method, std::string &body);
        std::string buildHttpResponse(const std::string &statusCode, const std::string &contentType, const std::string &body);
    };
}  // namespace network
