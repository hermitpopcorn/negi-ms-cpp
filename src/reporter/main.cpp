#include <iostream>
#include <curl/curl.h>
#include <time.h>
#include <algorithm>
#include <map>

#include "lib/network/requester.hpp"
#include "lib/sheet/client.hpp"
#include "lib/external/exec.hpp"
#include "discord.hpp"

int main(int argc, char *argv[])
{
    char *discordBotToken = std::getenv("DISCORD_BOT_TOKEN");
    if (discordBotToken == nullptr)
        throw std::runtime_error("DISCORD_BOT_TOKEN not found in env");
    char *discordChannelId = std::getenv("DISCORD_CHANNEL_ID");
    if (discordChannelId == nullptr)
        throw std::runtime_error("DISCORD_CHANNEL_ID not found in env");

    uint hours = 24 * 7;
    if (argc >= 2)
    {
        hours = std::stoi(argv[1]);
    }

    curl_global_init(CURL_GLOBAL_DEFAULT);

    try
    {
        // Get time point of 7 days ago
        auto oneWeekAgo = std::chrono::system_clock::now() - std::chrono::hours(hours);

        // Get transactions
        auto requester = std::make_shared<network::Requester>();
        auto exec = std::make_shared<external::ShellExec>();
        sheet::Client client(requester, exec);
        auto transactions = client.getTransactions();

        // Sort (newest first)
        std::sort(transactions.begin(), transactions.end(), [](sheet::Transaction a, sheet::Transaction b)
                  { return a.date > b.date; });

        // Get transactions of the past week
        int endIndex;
        for (endIndex = 0; endIndex < transactions.size(); ++endIndex)
        {
            if (transactions[endIndex].date < oneWeekAgo)
            {
                break;
            }

            // Give default category name if unspecified
            if (transactions[endIndex].category.length() < 1)
            {
                transactions[endIndex].category = "Uncategorized";
            }
        }

        // Calculate totals
        using TotalsByCurrency = std::map<std::string, int>;
        using TotalsByCategory = std::map<std::string, TotalsByCurrency>;
        TotalsByCategory totals;
        {
            for (int i = 0; i < endIndex; ++i)
            {
                const auto &t = transactions[i];
                if (totals.count(t.category) == 0)
                {
                    totals[t.category] = TotalsByCurrency{};
                }

                auto &byCategory = totals[t.category];
                if (byCategory.count(t.currency) == 0)
                {
                    byCategory[t.currency] = 0;
                }
                byCategory[t.currency] += t.amount;
            }
        }

        auto toNumericString = [](const int &numeric) -> std::string
        {
            std::string from = std::to_string(numeric);
            std::string to = "";
            int c = 0;
            for (int i = from.size(); i >= 0; --i)
            {
                to.insert(0, 1, from[i]);
                if (++c > 3 && i != 0)
                {
                    to.insert(0, 1, ',');
                    c = 0;
                }
            }
            return to;
        };

        std::vector<std::string> lines;
        for (auto &byCategory : totals)
        {
            std::string amounts = "";
            for (auto &byCurrency : byCategory.second)
            {
                amounts += toNumericString(std::abs(byCurrency.second)) + " " + byCurrency.first;
                amounts += " | ";
            }
            amounts.erase(amounts.size() - 3, 3);
            lines.push_back("**" + byCategory.first + "**: " + amounts);
        }

        std::string linesMerged;
        auto iter = lines.begin();
        while (true)
        {
            linesMerged += *iter;
            if (++iter != lines.end())
            {
                linesMerged += "\n";
            }
            else
            {
                break;
            }
        }

        reporter::Discord discord(requester, discordBotToken);
        discord.sendMessage(discordChannelId, linesMerged);
    }
    catch (std::exception &e)
    {
        std::cout << "Error: " << e.what() << std::endl;
    }

    curl_global_cleanup();
    return 0;
}
