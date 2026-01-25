#pragma once

#include <vector>
#include <map>
#include <string>
#include "lib/sheet.hpp"

namespace marksman
{
    std::string readCategoryMapFile();
    std::map<std::string, std::string> parseCategoryMap(const std::string &csvContent);
    std::vector<sheet::TransactionRow> matchSubjectToCategories(
        const std::vector<sheet::Transaction> &transactions,
        const std::map<std::string, std::string> &categoryMap);
}
