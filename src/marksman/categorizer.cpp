#include <fstream>
#include <sstream>
#include <map>
#include <memory>
#include <cstdlib>
#include "categorizer.hpp"

namespace marksman
{
    std::string readCategoryMapFile()
    {
        const char *categoryMapPath = std::getenv("CATEGORY_MAP_FILE");

        if (categoryMapPath == nullptr)
        {
            throw std::runtime_error("CATEGORY_MAP_FILE environment variable not set");
        }

        std::ifstream file(categoryMapPath);
        if (!file.is_open())
        {
            throw std::runtime_error(std::string("Could not open category map file: ") + categoryMapPath);
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }

    std::map<std::string, std::string> parseCategoryMap(const std::string &csvContent)
    {
        std::map<std::string, std::string> categoryMap;
        std::istringstream stream(csvContent);
        std::string line;

        while (std::getline(stream, line))
        {
            // Trim the line
            line.erase(0, line.find_first_not_of(" \t\r\n"));
            line.erase(line.find_last_not_of(" \t\r\n") + 1);

            // Skip empty lines
            if (line.empty())
            {
                continue;
            }

            // Split by comma
            size_t commaPos = line.find(',');
            if (commaPos != std::string::npos)
            {
                std::string key = line.substr(0, commaPos);
                std::string value = line.substr(commaPos + 1);

                // Trim key
                key.erase(0, key.find_first_not_of(" \t\r\n"));
                key.erase(key.find_last_not_of(" \t\r\n") + 1);

                // Trim value
                value.erase(0, value.find_first_not_of(" \t\r\n"));
                value.erase(value.find_last_not_of(" \t\r\n") + 1);

                categoryMap[key] = value;
            }
        }

        return categoryMap;
    }

    std::vector<sheet::TransactionRow> matchSubjectToCategories(
        const std::vector<sheet::Transaction> &transactions,
        const std::map<std::string, std::string> &categoryMap)
    {
        std::vector<sheet::TransactionRow> matchedValues;

        int rowNumber = 2; // Start from row 2 (A2)
        for (const auto &trx : transactions)
        {
            // Skip if already has category or no subject
            if (trx.subject.empty() || !trx.category.empty())
            {
                rowNumber++;
                continue;
            }

            auto mutableTrx = std::make_shared<sheet::Transaction>(trx);
            for (const auto &pair : categoryMap)
            {
                const std::string &keyword = pair.first;
                const std::string &category = pair.second;

                if (mutableTrx->subject.find(keyword) != std::string::npos)
                {
                    mutableTrx->category = category;
                    break;
                }
            }

            // Only include if a match was found
            if (!mutableTrx->category.empty())
            {
                matchedValues.push_back({mutableTrx, rowNumber});
            }

            rowNumber++;
        }

        return matchedValues;
    }
}
