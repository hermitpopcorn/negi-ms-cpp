#pragma once

#include <string>
#include <vector>

namespace network
{
    class RequesterInterface
    {
    public:
        virtual std::string getRequest(const std::string &url, const std::vector<std::string> &headers) = 0;
        virtual std::string postRequest(const std::string &url, const std::vector<std::string> &headers, const std::string &body) = 0;
        virtual std::string putRequest(const std::string &url, const std::vector<std::string> &headers, const std::string &body) = 0;
    };
}
