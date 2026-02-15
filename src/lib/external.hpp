#pragma once

#include <string>
#include <utility>

namespace external
{
    class ExecInterface
    {
      public:
        virtual ~ExecInterface() = default;
        virtual std::pair<int, std::string> execCommand(const std::string &command) = 0;
        virtual std::string googleOAuth(const std::string &scopes) = 0;
    };
}  // namespace external
