#include <iostream>
#include <vector>
#include <nlohmann/json.hpp>
#include "discord.hpp"
#include "lib/network.hpp"

namespace reporter
{
    Discord::Discord(std::shared_ptr<network::RequesterInterface> p_requester, std::string botToken)
        : mp_requester(std::move(p_requester)), m_botToken(botToken)
    {
    }

    void Discord::sendMessage(std::string channelId, std::string content)
    {
        std::string url = "https://discord.com/api/channels/" + channelId + "/messages";
        std::vector<std::string> headers = {
            "Authorization: Bot " + m_botToken,
            "Content-Type: application/json",
        };

        nlohmann::json body = nlohmann::json::object();
        body["content"] = content;

        mp_requester->postRequest(url, headers, body.dump());
    }
}
