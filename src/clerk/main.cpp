#include <fstream>
#include <iostream>
#include <memory>

#include "lib/network/http_server.hpp"

auto handler = [](const std::string &path, const std::string &method) -> network::HttpResponse {
    network::HttpResponse response;
    std::cout << "Request: " << method << " " << path << std::endl;

    if (path == "/")
    {
        std::ifstream file("./clerk-fe/index.html");
        if (file.is_open())
        {
            response.content = std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
            response.type = "text/html";
        }
    }
    else
    {
        std::ifstream file("./clerk-fe" + path);
        if (file.is_open())
        {
            response.content = std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        }
    }

    return response;
};


int main()
{
    auto server = std::make_shared<network::HttpServer>();
    server->setPort(8080);
    server->setRequestHandler(handler);

    server->start();
    while (server->isRunning())
    {
        server->acceptConnection();
    }
}
