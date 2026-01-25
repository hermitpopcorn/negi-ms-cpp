#include <iostream>
#include <memory>
#include <curl/curl.h>
#include "lib/sheet/client.hpp"
#include "lib/network/requester.hpp"
#include "lib/external/exec.hpp"
#include "duplifinder.hpp"
#include "categorizer.hpp"

void markDuplicates(sheet::Client &client, const std::vector<sheet::Transaction> &values)
{
    auto possibleDuplicates = marksman::findPossibleDuplicates(values);

    std::cout << "Found " << possibleDuplicates.size() << " possible duplicates" << std::endl;

    if (possibleDuplicates.empty())
    {
        return;
    }

    try
    {
        client.markDuplicatesInSheet(possibleDuplicates);
        std::cout << "Marked all of them as possible duplicates" << std::endl;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Marking error: " << e.what() << std::endl;
    }
}

void setCategories(sheet::Client &client, const std::vector<sheet::Transaction> &values)
{
    auto categoryMapCsv = marksman::readCategoryMapFile();
    auto categoryMap = marksman::parseCategoryMap(categoryMapCsv);
    auto matchedValues = marksman::matchSubjectToCategories(values, categoryMap);

    std::cout << "Found " << matchedValues.size() << " subject-to-category matches" << std::endl;

    if (matchedValues.empty())
    {
        return;
    }

    try
    {
        client.setCategoriesInSheet(matchedValues);
        std::cout << "Marked the categories for all of them" << std::endl;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Marking error: " << e.what() << std::endl;
    }
}

int main()
{
    curl_global_init(CURL_GLOBAL_DEFAULT);

    try
    {
        auto requester = std::make_shared<network::Requester>();
        auto shellExec = std::make_shared<external::ShellExec>();

        sheet::Client client(requester, shellExec);

        auto sheetValues = client.getTransactions();
        std::cout << "Fetched " << sheetValues.size() << " transactions from Google Sheets" << std::endl;

        markDuplicates(client, sheetValues);
        setCategories(client, sheetValues);

        curl_global_cleanup();
        return 0;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        curl_global_cleanup();
        return 1;
    }
}
