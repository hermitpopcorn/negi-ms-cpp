#pragma once

#include <memory>
#include "lib/sheet.hpp"
#include "lib/network.hpp"

namespace sheet
{
    class Client : public ClientInterface
    {
    private:
        std::shared_ptr<network::RequesterInterface> mp_requester;
        struct Token *mp_token;
        void getToken();
        void deleteToken();

    public:
        Client(std::shared_ptr<network::RequesterInterface> p_requester);
        ~Client();

        std::string getSheetData() override;
    };
}
