#pragma once

#include <string>
#include <vector>

namespace network
{
    class RequesterInterface
    {
    public:
        virtual std::string getRequest(std::string url, std::vector<std::string> headers) = 0;
        virtual std::string postRequest(std::string url, std::vector<std::string> headers, std::string body) = 0;
    };
}
