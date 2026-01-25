#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <memory>
#include "lib/sheet/client.hpp"
#include "marksman/duplifinder.hpp"
#include "marksman/categorizer.hpp"
#include "test_utils.hpp"

TEST(Marksman, FindDuplicatesWithinTwoDays)
{
    std::vector<sheet::Transaction> transactions = {
        // Two transactions on same account within 2 days, same amount
        {
            "Bank A",
            "Transaction 1",
            makeTimePoint(2025, 1, 1, 10, 0, 0),
            100000,
            "IDR",
            ""
        },
        {
            "Bank A",
            "Transaction 2",
            makeTimePoint(2025, 1, 2, 14, 0, 0),
            100000,
            "IDR",
            ""
        },
    };

    auto duplicates = marksman::findPossibleDuplicates(transactions);

    EXPECT_EQ(duplicates.size(), 1);
    EXPECT_EQ(duplicates[0].row, 3); // Row 3 (second transaction)
    EXPECT_EQ(duplicates[0].transaction->subject, "?dupof(2) Transaction 2");
    EXPECT_EQ(duplicates[0].transaction->account, "Bank A");
    EXPECT_EQ(duplicates[0].transaction->amount, 100000);
}

TEST(Marksman, IgnoreDuplicatesBeyondTwoDays)
{
    std::vector<sheet::Transaction> transactions = {
        // Two transactions on same account, same amount, but 3+ days apart
        {
            "Bank A",
            "Transaction 1",
            makeTimePoint(2025, 1, 1),
            100000,
            "IDR",
            ""
        },
        {
            "Bank A",
            "Transaction 2",
            makeTimePoint(2025, 1, 5),
            100000,
            "IDR",
            ""
        },
    };

    auto duplicates = marksman::findPossibleDuplicates(transactions);

    EXPECT_EQ(duplicates.size(), 0);
}

TEST(Marksman, IgnoreDifferentAmounts)
{
    std::vector<sheet::Transaction> transactions = {
        // Two transactions on same account, within 2 days, but different amounts
        {
            "Bank A",
            "Transaction 1",
            makeTimePoint(2025, 1, 1),
            100000,
            "IDR",
            ""
        },
        {
            "Bank A",
            "Transaction 2",
            makeTimePoint(2025, 1, 2),
            150000,
            "IDR",
            ""
        },
    };

    auto duplicates = marksman::findPossibleDuplicates(transactions);

    EXPECT_EQ(duplicates.size(), 0);
}

TEST(Marksman, IgnoreDifferentAccounts)
{
    std::vector<sheet::Transaction> transactions = {
        // Two transactions, different accounts, same amount, within 2 days
        {
            "Bank A",
            "Transaction 1",
            makeTimePoint(2025, 1, 1),
            100000,
            "IDR",
            ""
        },
        {
            "Bank B",
            "Transaction 2",
            makeTimePoint(2025, 1, 2),
            100000,
            "IDR",
            ""
        },
    };

    auto duplicates = marksman::findPossibleDuplicates(transactions);

    EXPECT_EQ(duplicates.size(), 0);
}

TEST(Marksman, SkipAlreadyMarkedAsDuplicate)
{
    std::vector<sheet::Transaction> transactions = {
        // First transaction is already marked as duplicate
        {
            "Bank A",
            "?dupof(10)Previous duplicate",
            makeTimePoint(2025, 1, 1),
            100000,
            "IDR",
            ""
        },
        {
            "Bank A",
            "Transaction 2",
            makeTimePoint(2025, 1, 2),
            100000,
            "IDR",
            ""
        },
    };

    auto duplicates = marksman::findPossibleDuplicates(transactions);

    EXPECT_EQ(duplicates.size(), 0);
}

TEST(Marksman, SkipBothMarkedAsNotDuplicate)
{
    std::vector<sheet::Transaction> transactions = {
        // Both marked with ! prefix (not duplicates)
        {
            "Bank A",
            "!Transaction 1",
            makeTimePoint(2025, 1, 1),
            100000,
            "IDR",
            ""
        },
        {
            "Bank A",
            "!Transaction 2",
            makeTimePoint(2025, 1, 2),
            100000,
            "IDR",
            ""
        },
    };

    auto duplicates = marksman::findPossibleDuplicates(transactions);

    EXPECT_EQ(duplicates.size(), 0);
}

TEST(Marksman, FlipWhenSecondMarkedAsNotDuplicate)
{
    std::vector<sheet::Transaction> transactions = {
        // Second transaction marked as not duplicate, so flip and mark first as duplicate
        {
            "Bank A",
            "Transaction 1",
            makeTimePoint(2025, 1, 1),
            100000,
            "IDR",
            ""
        },
        {
            "Bank A",
            "!Transaction 2",
            makeTimePoint(2025, 1, 2),
            100000,
            "IDR",
            ""
        },
    };

    auto duplicates = marksman::findPossibleDuplicates(transactions);

    EXPECT_EQ(duplicates.size(), 1);
    EXPECT_EQ(duplicates[0].row, 2); // First transaction marked as duplicate instead
    EXPECT_EQ(duplicates[0].transaction->subject, "?dupof(3) Transaction 1");
}

TEST(Marksman, MultipleGroupsByAmount)
{
    std::vector<sheet::Transaction> transactions = {
        // Group 1: Amount 100000, 2 transactions
        {
            "Bank A",
            "Trx 1.1",
            makeTimePoint(2025, 1, 1),
            100000,
            "IDR",
            ""
        },
        {
            "Bank A",
            "Trx 1.2",
            makeTimePoint(2025, 1, 2),
            100000,
            "IDR",
            ""
        },
        // Group 2: Amount 200000, 2 transactions
        {
            "Bank A",
            "Trx 2.1",
            makeTimePoint(2025, 1, 1),
            200000,
            "IDR",
            ""
        },
        {
            "Bank A",
            "Trx 2.2",
            makeTimePoint(2025, 1, 2),
            200000,
            "IDR",
            ""
        },
    };

    auto duplicates = marksman::findPossibleDuplicates(transactions);

    EXPECT_EQ(duplicates.size(), 2);
    // Check that we have duplicates from both groups
    EXPECT_TRUE(duplicates[0].transaction->subject.find("?dupof") != std::string::npos);
    EXPECT_TRUE(duplicates[1].transaction->subject.find("?dupof") != std::string::npos);
}

TEST(Marksman, PreservesOriginalSubjectAfterDuplicate)
{
    std::vector<sheet::Transaction> transactions = {
        {
            "Bank A",
            "Original subject text",
            makeTimePoint(2025, 1, 1),
            100000,
            "IDR",
            ""
        },
        {
            "Bank A",
            "Duplicate subject text",
            makeTimePoint(2025, 1, 2),
            100000,
            "IDR",
            ""
        },
    };

    auto duplicates = marksman::findPossibleDuplicates(transactions);

    EXPECT_EQ(duplicates.size(), 1);
    EXPECT_EQ(duplicates[0].transaction->subject, "?dupof(2) Duplicate subject text");
}

TEST(Marksman, EmptySubjectNoDuplicate)
{
    std::vector<sheet::Transaction> transactions = {
        {
            "Bank A",
            "",
            makeTimePoint(2025, 1, 1),
            100000,
            "IDR",
            ""
        },
        {
            "Bank A",
            "Some text",
            makeTimePoint(2025, 1, 2),
            100000,
            "IDR",
            ""
        },
    };

    auto duplicates = marksman::findPossibleDuplicates(transactions);

    EXPECT_EQ(duplicates.size(), 1);
    EXPECT_EQ(duplicates[0].transaction->subject, "?dupof(2) Some text");
}

TEST(Marksman, EdgeCase_AlmostThreeDaysApart)
{
    std::vector<sheet::Transaction> transactions = {
        {"Bank A",
         "Transaction 1",
         makeTimePoint(2025, 9, 6, 11, 43, 0),
         100000,
         "IDR",
         ""},
        {"Bank A",
         "Transaction 2",
         makeTimePoint(2025, 9, 9, 10, 35, 0),
         100000,
         "IDR",
         ""},
    };

    auto duplicates = marksman::findPossibleDuplicates(transactions);

    EXPECT_EQ(duplicates.size(), 0);
}
