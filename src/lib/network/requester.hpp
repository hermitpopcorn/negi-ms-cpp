#pragma once

#include "lib/network.hpp"

namespace network
{
    class Requester : public RequesterInterface
    {
    public:
        Requester();
        ~Requester();

        std::string getRequest(std::string url, std::vector<std::string> headers) override;
    };
}
