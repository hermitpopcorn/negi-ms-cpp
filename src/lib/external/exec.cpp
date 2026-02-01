#include <stdexcept>
#include <array>
#include "lib/external/exec.hpp"
#include <algorithm>

#define EXEC_OK (0)

namespace external
{
    std::pair<int, std::string> ShellExec::execCommand(const std::string &command)
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

    std::string ShellExec::googleOAuth(const std::string &scopes)
    {
        char *serviceFilePath = std::getenv("GOOGLE_APPLICATION_CREDENTIALS");
        if (serviceFilePath == nullptr)
            throw std::runtime_error("credentials file is unset");

        std::string command = "google-oauth2 " + std::string(serviceFilePath) + " " + scopes;
        std::pair<int, std::string> result = execCommand(command);
        if (result.first != EXEC_OK)
        {
            throw std::runtime_error("oauth2 failed (" + std::to_string(result.first) + "): " + result.second);
        }

        result.second.erase(std::remove(result.second.begin(), result.second.end(), '\n'), result.second.end());
        result.second.erase(std::remove(result.second.begin(), result.second.end(), '\r'), result.second.end());

        return result.second;
    }
}
