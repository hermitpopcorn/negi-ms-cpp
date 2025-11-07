#include <iostream>
#include <cstdlib>
#include <fstream>
#include "sheet.hpp"
#include "google/cloud/oauth2/access_token_generator.h"

namespace sheet
{
	Client::Client()
		: mp_token(nullptr)
	{
		getToken();
	}

	Client::~Client()
	{}

	struct Client::Token {
		google::cloud::AccessToken googleAccessToken;

		Token(google::cloud::AccessToken token)
			: googleAccessToken(token)
		{}
	};

	void Client::getToken()
	{
		// read credentials json file
		std::string credentialsJson;
		{
			char* serviceFilePath = std::getenv("GOOGLE_APPLICATION_CREDENTIALS");
			if (serviceFilePath == nullptr) throw std::runtime_error("credentials file is unset");
			std::ifstream file(serviceFilePath);
			if (!file) throw std::runtime_error("could not open credentials file");
			credentialsJson = std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
		}

		// authenticate and store token
		google::cloud::Options options;
		options.set<google::cloud::ScopesOption>(std::vector<std::string>{
    		"https://www.googleapis.com/auth/spreadsheets",
		});
		auto credentials = google::cloud::MakeServiceAccountCredentials(credentialsJson, options);
		auto generator = google::cloud::oauth2::MakeAccessTokenGenerator(*credentials);
		auto token = generator->GetToken();
		if (!token) throw std::runtime_error(token.status().message());
		mp_token = std::make_unique<Client::Token>(token.value());
	}
}
