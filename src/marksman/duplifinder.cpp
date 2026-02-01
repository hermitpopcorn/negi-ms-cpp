#include <algorithm>
#include <cmath>
#include <memory>
#include <map>
#include "duplifinder.hpp"

namespace marksman
{
    std::vector<sheet::TransactionRow> findPossibleDuplicates(const std::vector<sheet::Transaction> &transactions)
    {
        // Create a vector of pairs with row numbers and copies of transactions
        std::vector<std::pair<int, sheet::Transaction>> txnsWithRows;
        int rowNumber = 2; // Start from row 2 (A2)
        for (const auto &trx : transactions)
        {
            // Skip ones already marked as duplicate
            if (!trx.subject.empty() && trx.subject[0] == '?')
            {
                rowNumber++;
                continue;
            }
            txnsWithRows.push_back({rowNumber, trx});
            rowNumber++;
        }

        // Group by amount
        std::map<int, std::vector<size_t>> groupedByAmount;
        for (size_t i = 0; i < txnsWithRows.size(); ++i)
        {
            groupedByAmount[txnsWithRows[i].second.amount].push_back(i);
        }

        // Sort each group by date
        for (auto &pair : groupedByAmount)
        {
            std::sort(pair.second.begin(), pair.second.end(),
                      [&txnsWithRows](size_t a, size_t b)
                      {
                          return txnsWithRows[a].second.date < txnsWithRows[b].second.date;
                      });
        }

        // Find possible duplicates
        std::vector<sheet::TransactionRow> possibleDuplicates;
        for (const auto &group : groupedByAmount)
        {
            const auto &indices = group.second;
            for (size_t i = 0; i < indices.size() - 1; ++i)
            {
                const auto &current = txnsWithRows[indices[i]];
                const auto &next = txnsWithRows[indices[i + 1]];

                auto timeDiff = next.second.date - current.second.date;
                // Convert to seconds first, then to days
                auto secondsDiff = std::chrono::duration_cast<std::chrono::seconds>(timeDiff);
                // Use ceiling division to properly account for partial days
                int daysDiff = (secondsDiff.count() + 86399) / (24 * 3600);

                std::string account1 = current.second.account;
                std::string account2 = next.second.account;

                // Trim whitespace for comparison
                account1.erase(account1.find_last_not_of(" \t") + 1);
                account2.erase(account2.find_last_not_of(" \t") + 1);

                if (std::abs(daysDiff) <= 2 && account1 == account2)
                {
                    // Skip if both marked as not duplicate
                    bool currentMarkedNotDupe = !current.second.subject.empty() && current.second.subject[0] == '!';
                    bool nextMarkedNotDupe = !next.second.subject.empty() && next.second.subject[0] == '!';

                    if (currentMarkedNotDupe && nextMarkedNotDupe)
                    {
                        continue;
                    }

                    // If the latter is marked as not duplicate, flip
                    bool flip = !currentMarkedNotDupe && nextMarkedNotDupe;

                    // Check if we should flip based on time of day
                    if (!flip)
                    {
                        // Extract time of day (minutes and seconds)
                        auto currentTimeOfDay = std::chrono::duration_cast<std::chrono::seconds>(current.second.date.time_since_epoch()) % std::chrono::seconds(86400);
                        auto nextTimeOfDay = std::chrono::duration_cast<std::chrono::seconds>(next.second.date.time_since_epoch()) % std::chrono::seconds(86400);

                        auto currentMinutes = (currentTimeOfDay.count() / 60) % 60;
                        auto currentSeconds = currentTimeOfDay.count() % 60;
                        auto nextMinutes = (nextTimeOfDay.count() / 60) % 60;
                        auto nextSeconds = nextTimeOfDay.count() % 60;

                        // Flip if current has 00:00 and next has non-00 minutes/seconds
                        if (currentMinutes == 0 && currentSeconds == 0 && (nextMinutes != 0 || nextSeconds != 0))
                        {
                            flip = true;
                        }
                    }

                    int originalRow = flip ? next.first : current.first;
                    const sheet::Transaction &duplicateTxn = flip ? current.second : next.second;
                    int duplicateRow = flip ? current.first : next.first;

                    auto clonedDuplicate = std::make_shared<sheet::Transaction>(duplicateTxn);
                    std::string originalSubject = clonedDuplicate->subject;
                    if (!originalSubject.empty())
                    {
                        originalSubject = " " + originalSubject;
                    }
                    clonedDuplicate->subject = "?dupof(" + std::to_string(originalRow) + ")" + originalSubject;

                    possibleDuplicates.push_back({clonedDuplicate, duplicateRow});
                }
            }
        }

        return possibleDuplicates;
    }
}
