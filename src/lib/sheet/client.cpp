#include <nlohmann/json.hpp>
#include <cmath>
#include "lib/sheet/client.hpp"

namespace sheet
{
	struct Token
	{
		std::string accessToken;
	};

	static std::chrono::system_clock::time_point googleSheetsDateTimeToTimePoint(const double googleSheetsValue)
	{
		double integral = 0;
		double fraction = modf(googleSheetsValue, &integral);

		// Google Sheets uses 1899-12-30 as Day 0
		// 1899-12-30 to 1970-1-1 = 25569 days
		int daysSinceEpoch = integral - 25569;
		int minutes = fraction * (24 * 60);

		auto sinceEpoch = std::chrono::hours(daysSinceEpoch * 24) + std::chrono::minutes(minutes);
		return std::chrono::time_point<std::chrono::system_clock>(sinceEpoch);
	}

	Client::Client(std::shared_ptr<network::RequesterInterface> p_requester, std::shared_ptr<external::ExecInterface> p_exec)
		: mp_requester(std::move(p_requester)), mp_exec(std::move(p_exec)), mp_token(nullptr), m_sheetId("")
	{
		if (mp_requester == nullptr)
		{
			throw std::runtime_error("requester is null");
		}
		getToken();
	}

	Client::~Client()
	{
		deleteToken();
	}

	void Client::setSheetId(const std::string &sheetId)
	{
		m_sheetId = sheetId;
	}

	void Client::deleteToken()
	{
		if (mp_token != nullptr)
		{
			delete mp_token;
			mp_token = nullptr;
		}
	}

	void Client::getToken()
	{
		mp_token = new Token{mp_exec->googleOAuth("https://www.googleapis.com/auth/spreadsheets")};
	}

	std::vector<Transaction> Client::getTransactions()
	{
		if (mp_requester == nullptr)
		{
			throw std::runtime_error("requester is null");
		}

		if (mp_token == nullptr)
		{
			throw std::runtime_error("token is null");
		}

		auto fetchTransactionJson = [this]() -> std::string
		{
			std::string range = "Transactions!A2:F";
			std::vector<std::string> headers;
			headers.push_back("Authorization: Bearer " + mp_token->accessToken);
			headers.push_back("Content-Type: application/json");
			std::string url = "https://sheets.googleapis.com/v4/spreadsheets/" + m_sheetId + "/values/" + range + "?valueRenderOption=UNFORMATTED_VALUE";
			return mp_requester->getRequest(std::move(url), std::move(headers));
		};

		std::string response = fetchTransactionJson();

		auto parseJsonString = [](std::string &jsonString) -> std::vector<Transaction>
		{
			std::vector<Transaction> transactions;

			nlohmann::json json = nlohmann::json::parse(jsonString);
			for (auto &v : json["values"])
			{
				auto date = googleSheetsDateTimeToTimePoint(v[2]);

				auto getString = [](nlohmann::json &i) -> std::string
				{
					return i.is_null() ? "" : i;
				};
				auto getNumeric = [](nlohmann::json &i) -> int
				{
					if (i.is_string())
					{
						std::string s = i;
						if (s.length() < 1)
						{
							return 0;
						}
						return std::stoi(s);
					}

					if (i.is_null())
					{
						return 0;
					}

					return i;
				};

				transactions.emplace_back(Transaction{
					getString(v[0]),
					getString(v[1]),
					date,
					getNumeric(v[3]),
					getString(v[4]),
					getString(v[5]),
				});
			}

			return transactions;
		};

		return parseJsonString(response);
	}

	void Client::markDuplicatesInSheet(const std::vector<TransactionRow> &transactionRows)
	{
		if (mp_requester == nullptr)
		{
			throw std::runtime_error("requester is null");
		}

		if (mp_token == nullptr)
		{
			throw std::runtime_error("token is null");
		}

		for (const auto &trxRow : transactionRows)
		{
			// Update subject (column B)
			{
				std::string range = "Transactions!B" + std::to_string(trxRow.row) + ":B" + std::to_string(trxRow.row);
				std::string url = "https://sheets.googleapis.com/v4/spreadsheets/" + m_sheetId + "/values/" + range + "?valueInputOption=USER_ENTERED&includeValuesInResponse=0";

				std::vector<std::string> headers;
				headers.push_back("Authorization: Bearer " + mp_token->accessToken);
				headers.push_back("Content-Type: application/json");

				nlohmann::json valueRange = {
					{"range", range},
					{"values", nlohmann::json::array({nlohmann::json::array({trxRow.transaction->subject})})}};

				mp_requester->putRequest(url, headers, valueRange.dump());
			}

			// Update amount to 0 (column D)
			{
				std::string range = "Transactions!D" + std::to_string(trxRow.row) + ":D" + std::to_string(trxRow.row);
				std::string url = "https://sheets.googleapis.com/v4/spreadsheets/" + m_sheetId + "/values/" + range + "?valueInputOption=USER_ENTERED&includeValuesInResponse=0";

				std::vector<std::string> headers;
				headers.push_back("Authorization: Bearer " + mp_token->accessToken);
				headers.push_back("Content-Type: application/json");

				nlohmann::json valueRange = {
					{"range", range},
					{"values", nlohmann::json::array({nlohmann::json::array({"0"})})}};

				mp_requester->putRequest(url, headers, valueRange.dump());
			}
		}
	}

	void Client::setCategoriesInSheet(const std::vector<TransactionRow> &transactionRows)
	{
		if (mp_requester == nullptr)
		{
			throw std::runtime_error("requester is null");
		}

		if (mp_token == nullptr)
		{
			throw std::runtime_error("token is null");
		}

		for (const auto &trxRow : transactionRows)
		{
			// Update category (column F)
			std::string range = "Transactions!F" + std::to_string(trxRow.row) + ":F" + std::to_string(trxRow.row);
			std::string url = "https://sheets.googleapis.com/v4/spreadsheets/" + m_sheetId + "/values/" + range + "?valueInputOption=USER_ENTERED&includeValuesInResponse=0";

			std::vector<std::string> headers;
			headers.push_back("Authorization: Bearer " + mp_token->accessToken);
			headers.push_back("Content-Type: application/json");

			nlohmann::json valueRange = {
				{"range", range},
				{"values", nlohmann::json::array({nlohmann::json::array({trxRow.transaction->category})})}};

			mp_requester->putRequest(url, headers, valueRange.dump());
		}
	}
}
