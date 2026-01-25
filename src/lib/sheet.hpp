#pragma once

#include <string>
#include <chrono>
#include <memory>
#include "lib/network.hpp"

namespace sheet
{
	struct Transaction
	{
		std::string account;
		std::string subject;
		std::chrono::time_point<std::chrono::system_clock> date;
		int amount;
		std::string currency;
		std::string category;
	};

	struct TransactionRow
	{
		std::shared_ptr<Transaction> transaction;
		int row;
	};

	class ClientInterface
	{
	public:
		virtual void setSheetId(const std::string &sheetId) = 0;
		virtual std::vector<Transaction> getTransactions() = 0;
		virtual void markDuplicatesInSheet(const std::vector<TransactionRow> &transactionRows) = 0;
		virtual void setCategoriesInSheet(const std::vector<TransactionRow> &transactionRows) = 0;
	};
}
