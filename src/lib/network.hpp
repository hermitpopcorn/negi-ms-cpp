#pragma once

#include <string>
#include <vector>
#include <functional>

namespace network
{
    class RequesterInterface
    {
      public:
        virtual ~RequesterInterface() = default;
        virtual std::string getRequest(const std::string &url,
                                       const std::vector<std::string> &headers) = 0;
        virtual std::string postRequest(const std::string &url,
                                        const std::vector<std::string> &headers,
                                        const std::string &body) = 0;
        virtual std::string putRequest(const std::string &url,
                                       const std::vector<std::string> &headers,
                                       const std::string &body) = 0;
    };

    struct HttpResponse {
      std::string content;
      std::string type;
    };
    using RequestHandler = std::function<HttpResponse(const std::string &path, const std::string &method)>;

    class HttpServerInterface
    {
      public:
        virtual ~HttpServerInterface() = default;
        virtual void setPort(int port) = 0;
        virtual void setRequestHandler(RequestHandler handler) = 0;
        virtual void start() = 0;
        virtual void stop() = 0;
        virtual bool isRunning() const = 0;
        virtual bool acceptConnection() = 0;
    };
}  // namespace network
