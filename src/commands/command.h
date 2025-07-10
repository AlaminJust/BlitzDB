#pragma once

#include <string>

namespace blitzdb {

    class Command {
    public:
        virtual std::string execute() = 0;
        virtual ~Command() = default;
    };

} // namespace blitzdb