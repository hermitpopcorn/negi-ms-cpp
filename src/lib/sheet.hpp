#pragma once

#include <string>
#include <chrono>
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

	class ClientInterface
	{
	public:
		virtual std::vector<Transaction> getTransactions() = 0;
	};
}
