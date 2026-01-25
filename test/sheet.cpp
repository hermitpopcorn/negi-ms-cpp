#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "lib/sheet/client.hpp"
#include "lib/network/requester.hpp"
#include "lib/external/exec.hpp"
#include "test_utils.hpp"

class MockRequester : public network::Requester
{
public:
    MOCK_METHOD(std::string, getRequest, (const std::string &url, const std::vector<std::string> &headers), ());
};

class MockExec : public external::ShellExec
{
public:
    MOCK_METHOD(std::string, googleOAuth, (const std::string &scopes), ());
};

TEST(Sheet, ClientGetTransactions)
{
    auto mockedRequester = std::make_shared<MockRequester>();
    auto mockedExec = std::make_shared<MockExec>();

    EXPECT_CALL(*mockedExec, googleOAuth)
        .Times(testing::Exactly(1))
        .WillRepeatedly(testing::Return("test-access-token"));

    EXPECT_CALL(*mockedRequester, getRequest(testing::_, testing::Contains("Authorization: Bearer test-access-token")))
        .Times(testing::Exactly(1))
        .WillRepeatedly(testing::Return("{ \"values\": [[\"account\", \"subject\", 45658.0, 250000, \"IDR\", \"category\"]] }"));

    auto client = sheet::Client(mockedRequester, mockedExec);
    std::vector<sheet::Transaction> trxs = client.getTransactions();

    EXPECT_EQ(trxs.size(), 1);

    EXPECT_EQ(trxs[0].account, "account");
    EXPECT_EQ(trxs[0].subject, "subject");
    EXPECT_EQ(trxs[0].date, makeTimePoint(2025, 1, 1));
    EXPECT_EQ(trxs[0].amount, 250000);
    EXPECT_EQ(trxs[0].currency, "IDR");
    EXPECT_EQ(trxs[0].category, "category");
}

TEST(Sheet, GoogleSheetsDateTimeParsing)
{
    auto mockedRequester = std::make_shared<MockRequester>();
    auto mockedExec = std::make_shared<MockExec>();

    EXPECT_CALL(*mockedExec, googleOAuth)
        .WillRepeatedly(testing::Return("test-token"));

    // 45657.5 = 2024-12-31 12:00:00
    // 45658.25 = 2025-01-01 06:00:00
    // 45659.75 = 2025-01-02 18:00:00
    EXPECT_CALL(*mockedRequester, getRequest(testing::_, testing::_))
        .WillOnce(testing::Return(R"({
            "values": [
                ["Account1", "Subject1", 45657.5, 100000, "IDR", "Food"],
                ["Account2", "Subject2", 45658.25, 250000, "IDR", "Transport"],
                ["Account3", "Subject3", 45659.75, 500000, "IDR", "Utilities"]
            ]
        })"));

    auto client = sheet::Client(mockedRequester, mockedExec);
    std::vector<sheet::Transaction> trxs = client.getTransactions();

    EXPECT_EQ(trxs.size(), 3);

    EXPECT_EQ(trxs[0].account, "Account1");
    EXPECT_EQ(trxs[0].subject, "Subject1");
    EXPECT_EQ(trxs[0].date, makeTimePoint(2024, 12, 31, 12, 0, 0));
    EXPECT_EQ(trxs[0].amount, 100000);
    EXPECT_EQ(trxs[0].currency, "IDR");
    EXPECT_EQ(trxs[0].category, "Food");

    EXPECT_EQ(trxs[1].account, "Account2");
    EXPECT_EQ(trxs[1].subject, "Subject2");
    EXPECT_EQ(trxs[1].date, makeTimePoint(2025, 1, 1, 6, 0, 0));
    EXPECT_EQ(trxs[1].amount, 250000);
    EXPECT_EQ(trxs[1].currency, "IDR");
    EXPECT_EQ(trxs[1].category, "Transport");

    EXPECT_EQ(trxs[2].account, "Account3");
    EXPECT_EQ(trxs[2].subject, "Subject3");
    EXPECT_EQ(trxs[2].date, makeTimePoint(2025, 1, 2, 18, 0, 0));
    EXPECT_EQ(trxs[2].amount, 500000);
    EXPECT_EQ(trxs[2].currency, "IDR");
    EXPECT_EQ(trxs[2].category, "Utilities");
}
