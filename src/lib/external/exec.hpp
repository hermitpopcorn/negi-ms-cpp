#pragma once

#include "lib/external.hpp"

namespace external
{
    class ShellExec : public ExecInterface
    {
    public:
        std::pair<int, std::string> execCommand(const std::string &command) override;
        std::string googleOAuth(const std::string &scopes) override;
    };
}
