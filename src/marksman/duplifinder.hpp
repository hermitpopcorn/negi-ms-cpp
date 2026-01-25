#pragma once

#include <vector>
#include "lib/sheet.hpp"

namespace marksman
{
    std::vector<sheet::TransactionRow> findPossibleDuplicates(const std::vector<sheet::Transaction> &transactions);
}
