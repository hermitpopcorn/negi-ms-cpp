#pragma once

#include <string>
#include <vector>

namespace network
{
    class RequesterInterface
    {
    public:
        virtual std::string getRequest(std::string url, std::vector<std::string> headers) = 0;
    };
}
