#pragma once

#include <memory>
#include "lib/sheet.hpp"
#include "lib/network.hpp"
#include "lib/external.hpp"

namespace sheet
{
    class Client : public ClientInterface
    {
    private:
        std::shared_ptr<network::RequesterInterface> mp_requester;
        std::shared_ptr<external::ExecInterface> mp_exec;
        struct Token *mp_token;
        std::string m_sheetId;
        void getToken();
        void deleteToken();

    public:
        Client(std::shared_ptr<network::RequesterInterface> p_requester, std::shared_ptr<external::ExecInterface> p_exec);
        ~Client();

        void setSheetId(const std::string &sheetId) override;
        std::vector<Transaction> getTransactions() override;
        void markDuplicatesInSheet(const std::vector<TransactionRow> &transactionRows) override;
        void setCategoriesInSheet(const std::vector<TransactionRow> &transactionRows) override;
    };
}
