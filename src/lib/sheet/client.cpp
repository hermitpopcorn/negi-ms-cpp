#include <stdexcept>
#include <array>
#include <nlohmann/json.hpp>
#include <cmath>
#include "lib/sheet/client.hpp"

namespace sheet
{
	struct Token
	{
		std::string accessToken;
	};

	Client::Client(std::shared_ptr<network::RequesterInterface> p_requester)
		: mp_requester(std::move(p_requester)), mp_token(nullptr)
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

	void Client::deleteToken()
	{
		if (mp_token != nullptr)
		{
			delete mp_token;
			mp_token = nullptr;
		}
	}

#define EXEC_OK (0)
	std::pair<int, std::string> execExternal(const std::string &command)
	{
		// redirect stderr to stdout
		std::string realCommand = command + " 2>&1";

		std::string output = "";
		int waitStatus = -1;

		FILE *pipe = popen(realCommand.c_str(), "r");
		if (!pipe)
		{
			throw std::runtime_error("popen() failed");
		}

		std::array<char, 128> buffer;
		try
		{
			while (fgets(buffer.data(), buffer.size(), pipe) != nullptr)
			{
				output += buffer.data();
			}

			waitStatus = pclose(pipe);
		}
		catch (...)
		{
			pclose(pipe);
			throw;
		}

		return {waitStatus, output};
	}

	void Client::getToken()
	{
		char *serviceFilePath = std::getenv("GOOGLE_APPLICATION_CREDENTIALS");
		if (serviceFilePath == nullptr)
			throw std::runtime_error("credentials file is unset");

		std::string test_command = "google-oauth2 " + std::string(serviceFilePath) + " https://www.googleapis.com/auth/spreadsheets";
		std::pair<int, std::string> result = execExternal(test_command);
		if (result.first != EXEC_OK)
		{
			throw std::runtime_error("oauth2 failed (" + std::to_string(result.first) + "): " + result.second);
		}

		mp_token = new Token{result.second};
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
			std::string spreadsheetId = "1cK1dA50bW6_AyRA2ScDozwVg3IB_lHwBIP1xAqNwjXw";
			std::string range = "Transactions!A2:F";
			std::vector<std::string> headers;
			headers.push_back("Authorization: Bearer " + mp_token->accessToken);
			headers.push_back("Content-Type: application/json");
			std::string url = "https://sheets.googleapis.com/v4/spreadsheets/" + spreadsheetId + "/values/" + range + "?valueRenderOption=UNFORMATTED_VALUE";
			return mp_requester->getRequest(std::move(url), std::move(headers));
		};

		std::string response = fetchTransactionJson();

		auto parseJsonString = [](std::string &jsonString) -> std::vector<Transaction>
		{
			std::vector<Transaction> transactions;

			nlohmann::json json = nlohmann::json::parse(jsonString);
			for (auto &v : json["values"])
			{
				double d = v[2];
				double integral = 0;
				double fraction = modf(d, &integral);

				// Google Sheets uses 1899-12-30 as Day 0
				// 1899-12-30 to 1970-1-1 = 25569 days
				int daysSinceEpoch = integral - 25569;

				int minutes = fraction * (24 * 60);

				auto sinceEpoch = std::chrono::hours(daysSinceEpoch * 24) + std::chrono::minutes(minutes);
				auto date = std::chrono::time_point<std::chrono::system_clock>(sinceEpoch);

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
}
