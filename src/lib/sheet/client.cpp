#include <stdexcept>
#include <array>
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

		std::string test_command = "./google-oauth2 " + std::string(serviceFilePath) + " https://www.googleapis.com/auth/spreadsheets";
		std::pair<int, std::string> result = execExternal(test_command);
		if (result.first != EXEC_OK)
		{
			throw std::runtime_error("oauth2 failed (" + std::to_string(result.first) + "): " + result.second);
		}

		mp_token = new Token{result.second};
	}

	std::string Client::getSheetData()
	{
		if (mp_requester == nullptr)
		{
			throw std::runtime_error("requester is null");
		}

		if (mp_token == nullptr)
		{
			throw std::runtime_error("token is null");
		}

		std::string spreadsheetId = "1cK1dA50bW6_AyRA2ScDozwVg3IB_lHwBIP1xAqNwjXw";
		std::string range = "Transactions!A2:F";
		std::vector<std::string> headers;
		headers.push_back("Authorization: Bearer " + mp_token->accessToken);
		headers.push_back("Content-Type: application/json");
		std::string url = "https://sheets.googleapis.com/v4/spreadsheets/" + spreadsheetId + "/values/" + range + "?valueRenderOption=UNFORMATTED_VALUE";
		return mp_requester->getRequest(std::move(url), std::move(headers));
	}
}
