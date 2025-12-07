#pragma once

#include <utility>
#include <string>

namespace external
{
    class ExecInterface
    {
    public:
        virtual std::pair<int, std::string> execCommand(const std::string &command) = 0;
        virtual std::string googleOAuth(const std::string &scopes) = 0;
    };
}
