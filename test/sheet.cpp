#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "lib/sheet/client.hpp"
#include "lib/network/requester.hpp"
#include "lib/external/exec.hpp"

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

std::chrono::system_clock::time_point makeTimePoint(int year, int month, int day, int hour = 0, int minute = 0, int second = 0)
{
    std::tm t = {};
    t.tm_year = year - 1900;
    t.tm_mon = month - 1;
    t.tm_mday = day;
    t.tm_hour = hour;
    t.tm_min = minute;
    t.tm_sec = second;
    t.tm_isdst = -1;

    std::time_t tt = std::mktime(&t);

    return std::chrono::system_clock::from_time_t(tt);
}

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
