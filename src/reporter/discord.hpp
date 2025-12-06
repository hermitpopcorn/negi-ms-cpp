#pragma once

#include <string>
#include <memory>
#include "lib/network.hpp"

namespace reporter
{
    class Discord
    {
    private:
        std::shared_ptr<network::RequesterInterface> mp_requester;
        std::string m_botToken;

    public:
        Discord(std::shared_ptr<network::RequesterInterface> p_requester, std::string botToken);

        void sendMessage(std::string channelId, std::string content);
    };
}
