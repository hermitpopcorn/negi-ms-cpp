#pragma once

#include "lib/network.hpp"

namespace network
{
    class Requester : public RequesterInterface
    {
    public:
        Requester();
        ~Requester();

        std::string getRequest(const std::string &url, const std::vector<std::string> &headers) override;
        std::string postRequest(const std::string &url, const std::vector<std::string> &headers, const std::string &body) override;
        std::string putRequest(const std::string &url, const std::vector<std::string> &headers, const std::string &body) override;
    };
}
