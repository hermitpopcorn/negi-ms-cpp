#include <gtest/gtest.h>
#include <memory>
#include "marksman/categorizer.hpp"
#include "lib/sheet.hpp"
#include "test_utils.hpp"

const std::string MOCK_CATEGORY_MAP = R"(関西電力,Services
喜久屋書店,Manga
デイリーヤマザキ,Food
STEAMGAMES.COM,Games
)";

class CategorizerTest : public ::testing::Test
{
protected:
    // Helper to create a transaction
    sheet::Transaction createTransaction(
        const std::string &account,
        const std::string &subject,
        const std::string &category = "")
    {
        return {
            account,
            subject,
            makeTimePoint(2026, 1, 25, 12, 0, 0),
            10000,
            "JPY",
            category};
    }
};

// ============ Parse Category Map Tests ============

TEST_F(CategorizerTest, ParseCategoryMapCorrectlyParsesCsv)
{
    auto categoryMap = marksman::parseCategoryMap(MOCK_CATEGORY_MAP);

    EXPECT_EQ(categoryMap.size(), 4);
    EXPECT_EQ(categoryMap["関西電力"], "Services");
    EXPECT_EQ(categoryMap["喜久屋書店"], "Manga");
    EXPECT_EQ(categoryMap["デイリーヤマザキ"], "Food");
    EXPECT_EQ(categoryMap["STEAMGAMES.COM"], "Games");
}

TEST_F(CategorizerTest, ParseCategoryMapIgnoresEmptyLines)
{
    std::string csvWithEmptyLines = R"(関西電力,Services

喜久屋書店,Manga

)";

    auto categoryMap = marksman::parseCategoryMap(csvWithEmptyLines);

    EXPECT_EQ(categoryMap.size(), 2);
    EXPECT_EQ(categoryMap["関西電力"], "Services");
    EXPECT_EQ(categoryMap["喜久屋書店"], "Manga");
}

TEST_F(CategorizerTest, ParseCategoryMapTrimsWhitespace)
{
    std::string csvWithWhitespace = R"(  関西電力  ,  Services  
喜久屋書店,Manga  )";

    auto categoryMap = marksman::parseCategoryMap(csvWithWhitespace);

    EXPECT_EQ(categoryMap.size(), 2);
    EXPECT_EQ(categoryMap["関西電力"], "Services");
    EXPECT_EQ(categoryMap["喜久屋書店"], "Manga");
}

TEST_F(CategorizerTest, ParseCategoryMapHandlesEmptyCsv)
{
    auto categoryMap = marksman::parseCategoryMap("");

    EXPECT_EQ(categoryMap.size(), 0);
}

// ============ Match Subject to Categories Tests ============

TEST_F(CategorizerTest, MatchesSubjectWithExactKeyword)
{
    std::vector<sheet::Transaction> transactions = {
        createTransaction("Account1", "Payment to 関西電力")};

    auto categoryMap = marksman::parseCategoryMap(MOCK_CATEGORY_MAP);
    auto result = marksman::matchSubjectToCategories(transactions, categoryMap);

    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].transaction->category, "Services");
    EXPECT_EQ(result[0].transaction->subject, "Payment to 関西電力");
    EXPECT_EQ(result[0].row, 2);
}

TEST_F(CategorizerTest, MatchesMultipleTransactionsWithDifferentCategories)
{
    std::vector<sheet::Transaction> transactions = {
        createTransaction("Account1", "Buy from 喜久屋書店"),
        createTransaction("Account1", "Lunch at デイリーヤマザキ"),
        createTransaction("Account1", "Game on STEAMGAMES.COM")};

    auto categoryMap = marksman::parseCategoryMap(MOCK_CATEGORY_MAP);
    auto result = marksman::matchSubjectToCategories(transactions, categoryMap);

    EXPECT_EQ(result.size(), 3);
    EXPECT_EQ(result[0].transaction->category, "Manga");
    EXPECT_EQ(result[0].row, 2);
    EXPECT_EQ(result[1].transaction->category, "Food");
    EXPECT_EQ(result[1].row, 3);
    EXPECT_EQ(result[2].transaction->category, "Games");
    EXPECT_EQ(result[2].row, 4);
}

TEST_F(CategorizerTest, DoesNotMatchTransactionWithNoKeyword)
{
    std::vector<sheet::Transaction> transactions = {
        createTransaction("Account1", "Random payment")};

    auto categoryMap = marksman::parseCategoryMap(MOCK_CATEGORY_MAP);
    auto result = marksman::matchSubjectToCategories(transactions, categoryMap);

    EXPECT_EQ(result.size(), 0);
}

TEST_F(CategorizerTest, SkipsTransactionWithEmptySubject)
{
    std::vector<sheet::Transaction> transactions = {
        createTransaction("Account1", "")};

    auto categoryMap = marksman::parseCategoryMap(MOCK_CATEGORY_MAP);
    auto result = marksman::matchSubjectToCategories(transactions, categoryMap);

    EXPECT_EQ(result.size(), 0);
}

TEST_F(CategorizerTest, SkipsTransactionWithExistingCategory)
{
    std::vector<sheet::Transaction> transactions = {
        createTransaction("Account1", "Payment to 関西電力", "AlreadySet")};

    auto categoryMap = marksman::parseCategoryMap(MOCK_CATEGORY_MAP);
    auto result = marksman::matchSubjectToCategories(transactions, categoryMap);

    EXPECT_EQ(result.size(), 0);
}

TEST_F(CategorizerTest, OnlyIncludesMatchedTransactions)
{
    std::vector<sheet::Transaction> transactions = {
        createTransaction("Account1", "Random payment"),
        createTransaction("Account1", "Buy from 喜久屋書店"),
        createTransaction("Account1", "Another random payment"),
        createTransaction("Account1", "Lunch at デイリーヤマザキ")};

    auto categoryMap = marksman::parseCategoryMap(MOCK_CATEGORY_MAP);
    auto result = marksman::matchSubjectToCategories(transactions, categoryMap);

    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(result[0].transaction->category, "Manga");
    EXPECT_EQ(result[0].row, 3);
    EXPECT_EQ(result[1].transaction->category, "Food");
    EXPECT_EQ(result[1].row, 5);
}

TEST_F(CategorizerTest, PreservesSubjectAfterMatching)
{
    std::vector<sheet::Transaction> transactions = {
        createTransaction("Account1", "Payment to 喜久屋書店 for books")};

    auto categoryMap = marksman::parseCategoryMap(MOCK_CATEGORY_MAP);
    auto result = marksman::matchSubjectToCategories(transactions, categoryMap);

    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].transaction->subject, "Payment to 喜久屋書店 for books");
    EXPECT_EQ(result[0].transaction->category, "Manga");
}

TEST_F(CategorizerTest, MatchesFirstKeywordFound)
{
    // If subject contains multiple keywords, should match the first one found in the map
    std::vector<sheet::Transaction> transactions = {
        createTransaction("Account1", "Game at STEAMGAMES.COM and 喜久屋書店")};

    auto categoryMap = marksman::parseCategoryMap(MOCK_CATEGORY_MAP);
    auto result = marksman::matchSubjectToCategories(transactions, categoryMap);

    EXPECT_EQ(result.size(), 1);
    // Should match one of them (depends on map iteration order)
    EXPECT_TRUE(result[0].transaction->category == "Games" ||
                result[0].transaction->category == "Manga");
}

TEST_F(CategorizerTest, CaseSensitiveMatching)
{
    std::vector<sheet::Transaction> transactions = {
        createTransaction("Account1", "Payment to steamgames.com") // lowercase
    };

    auto categoryMap = marksman::parseCategoryMap(MOCK_CATEGORY_MAP);
    auto result = marksman::matchSubjectToCategories(transactions, categoryMap);

    // Should not match because STEAMGAMES.COM is uppercase
    EXPECT_EQ(result.size(), 0);
}

TEST_F(CategorizerTest, HandlesEmptyCategoryMap)
{
    std::vector<sheet::Transaction> transactions = {
        createTransaction("Account1", "Payment to 関西電力")};

    auto categoryMap = marksman::parseCategoryMap("");
    auto result = marksman::matchSubjectToCategories(transactions, categoryMap);

    EXPECT_EQ(result.size(), 0);
}

TEST_F(CategorizerTest, CorrectRowNumbersWithSkips)
{
    std::vector<sheet::Transaction> transactions = {
        createTransaction("Account1", "Payment to 関西電力"),      // Row 2 - matches
        createTransaction("Account1", "Random payment"),           // Row 3 - no match
        createTransaction("Account1", "Buy from 喜久屋書店"),      // Row 4 - matches
        createTransaction("Account1", "Another random payment"),   // Row 5 - no match
        createTransaction("Account1", "Lunch at デイリーヤマザキ") // Row 6 - matches
    };

    auto categoryMap = marksman::parseCategoryMap(MOCK_CATEGORY_MAP);
    auto result = marksman::matchSubjectToCategories(transactions, categoryMap);

    EXPECT_EQ(result.size(), 3);
    EXPECT_EQ(result[0].row, 2);
    EXPECT_EQ(result[0].transaction->category, "Services");
    EXPECT_EQ(result[1].row, 4);
    EXPECT_EQ(result[1].transaction->category, "Manga");
    EXPECT_EQ(result[2].row, 6);
    EXPECT_EQ(result[2].transaction->category, "Food");
}

TEST_F(CategorizerTest, PartialKeywordMatch)
{
    std::vector<sheet::Transaction> transactions = {
        createTransaction("Account1", "電力 related payment") // Contains "電力" which is part of "関西電力"
    };

    auto categoryMap = marksman::parseCategoryMap(MOCK_CATEGORY_MAP);
    auto result = marksman::matchSubjectToCategories(transactions, categoryMap);

    // Should not match because we search for full keyword "関西電力"
    EXPECT_EQ(result.size(), 0);
}
