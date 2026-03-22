#include <fstream>
#include <iostream>
#include <memory>
#include <ctime>
#include <nlohmann/json.hpp>

#include "lib/network/http_server.hpp"
#include "lib/sheet/client.hpp"
#include <lib/network/requester.hpp>
#include <lib/external/exec.hpp>

std::string sheetId;
std::string password;

namespace sheet
{
    void from_json(const nlohmann::json& j, Transaction& trx) {
        j.at("account").get_to(trx.account);
        j.at("subject").get_to(trx.subject);
        j.at("amount").get_to(trx.amount);

        std::string ts = j.at("datetime").get<std::string>();
        std::tm tm = {};
        strptime(ts.c_str(), "%Y-%m-%dT%H:%M:%SZ", &tm);
        trx.date = std::chrono::system_clock::from_time_t(std::mktime(&tm));
    }
}

auto handler = [](const std::string &path, const std::string &method, const std::string &body) -> network::HttpResponse {
    network::HttpResponse response;
    std::cout << "Request: " << method << " " << path << std::endl;

    response.code = 404;

    if (method == "GET") {
        if (path == "/")
        {
            std::ifstream file("./clerk-fe/index.html");
            if (file.is_open())
            {
                response.code = 200;
                response.content = std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
                response.type = "text/html";
            }
        }
        else
        {
            std::ifstream file("./clerk-fe" + path);
            if (file.is_open())
            {
                response.code = 200;
                response.content = std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
            }
        }
    } else if (method == "POST") {
        // ignore path
        do {
            sheet::Transaction trx;
            auto j_body = nlohmann::json::parse(body);

            std::string r_password;
            j_body.at("password").get_to(r_password);
            if (r_password != password) {
                response.code = 401;
                response.content = "";
                response.type = "";
                break;
            }

            j_body.get_to(trx);

            auto requester = std::make_shared<network::Requester>();
            auto exec = std::make_shared<external::ShellExec>();
            sheet::Client client(requester, exec);
            client.setSheetId(sheetId);
            client.addTransaction(trx);

            response.code = 200;
            response.content = "";
            response.type = "";
        } while (false);
    }


    return response;
};


int main()
{
    char* env_sheetId = std::getenv("SHEET_ID");
    if (env_sheetId == nullptr)
        throw std::runtime_error("SHEET_ID not found in env");
    sheetId = env_sheetId;

    char* env_password = std::getenv("PASSWORD");
    if (env_password == nullptr)
        throw std::runtime_error("PASSWORD not found in env");
    password = env_password;

    auto server = std::make_shared<network::HttpServer>();
    server->setPort(8080);
    server->setRequestHandler(handler);

    server->start();
    while (server->isRunning())
    {
        server->acceptConnection();
    }
}
