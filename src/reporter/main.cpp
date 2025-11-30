#include <iostream>
#include <curl/curl.h>

#include "lib/network/requester.hpp"
#include "lib/sheet/client.hpp"

int main()
{
    curl_global_init(CURL_GLOBAL_DEFAULT);

    std::cout << "Start" << std::endl;
    try
    {
        sheet::Client client(std::make_shared<network::Requester>());
        std::cout << client.getSheetData() << std::endl;
    }
    catch (std::exception &e)
    {
        std::cout << "Error: " << e.what() << std::endl;
    }

    curl_global_cleanup();
    return 0;
}
