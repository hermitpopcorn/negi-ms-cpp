#pragma once

#include "lib/network.hpp"

namespace sheet
{
	class ClientInterface
	{
	public:
		virtual std::string getSheetData() = 0;
	};
}
