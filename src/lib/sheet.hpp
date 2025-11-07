#pragma once

#include <memory>

namespace sheet
{
	class Client {
		private:
			struct Token;
			std::unique_ptr<Token> mp_token;
			void getToken();
		public:
			Client();
			~Client();
	};
}
