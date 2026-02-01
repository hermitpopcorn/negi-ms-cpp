#include <iostream>
#include <memory>
#include <curl/curl.h>
#include <chrono>
#include <iomanip>
#include <sstream>
#include "lib/sheet/client.hpp"
#include "lib/network/requester.hpp"
#include "lib/external/exec.hpp"
#include "duplifinder.hpp"
#include "categorizer.hpp"

std::string getCurrentTimestampUTC()
{
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    std::tm *tm_info = std::gmtime(&time_t_now);

    std::ostringstream oss;
    oss << std::put_time(tm_info, "[%Y-%m-%dT%H:%M:%SZ]");
    return oss.str();
}

void markDuplicates(sheet::Client &client, const std::vector<sheet::Transaction> &values)
{
    auto possibleDuplicates = marksman::findPossibleDuplicates(values);

    std::cout << getCurrentTimestampUTC() << " Found " << possibleDuplicates.size() << " possible duplicates" << std::endl;

    if (possibleDuplicates.empty())
    {
        return;
    }

    try
    {
        client.markDuplicatesInSheet(possibleDuplicates);
        std::cout << getCurrentTimestampUTC() << " Marked all of them as possible duplicates" << std::endl;
    }
    catch (const std::exception &e)
    {
        std::cerr << getCurrentTimestampUTC() << " Marking error: " << e.what() << std::endl;
    }
}

void setCategories(sheet::Client &client, const std::vector<sheet::Transaction> &values)
{
    auto categoryMapCsv = marksman::readCategoryMapFile();
    auto categoryMap = marksman::parseCategoryMap(categoryMapCsv);
    auto matchedValues = marksman::matchSubjectToCategories(values, categoryMap);

    std::cout << getCurrentTimestampUTC() << " Found " << matchedValues.size() << " subject-to-category matches" << std::endl;

    if (matchedValues.empty())
    {
        return;
    }

    try
    {
        client.setCategoriesInSheet(matchedValues);
        std::cout << getCurrentTimestampUTC() << " Marked the categories for all of them" << std::endl;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Marking error: " << e.what() << std::endl;
    }
}

int main()
{
    char *sheetId = std::getenv("SHEET_ID");
    if (sheetId == nullptr)
        throw std::runtime_error("SHEET_ID not found in env");

    curl_global_init(CURL_GLOBAL_DEFAULT);

    try
    {
        auto requester = std::make_shared<network::Requester>();
        auto shellExec = std::make_shared<external::ShellExec>();

        sheet::Client client(requester, shellExec);
        client.setSheetId(sheetId);

        auto sheetValues = client.getTransactions();
        std::cout << getCurrentTimestampUTC() << " Fetched " << sheetValues.size() << " transactions from Google Sheets" << std::endl;

        markDuplicates(client, sheetValues);
        setCategories(client, sheetValues);

        curl_global_cleanup();
        return 0;
    }
    catch (const std::exception &e)
    {
        std::cerr << getCurrentTimestampUTC() << " Error: " << e.what() << std::endl;
        curl_global_cleanup();
        return 1;
    }
}
