#pragma once
#include "command.h"
#include "../core/storage/in_memory.h"

namespace blitzdb {

    class GetCommand : public Command {
    public:
        GetCommand(InMemoryStorage& storage, const std::string& key);
        std::string execute() override;

    private:
        InMemoryStorage& storage_;
        std::string key_;
    };

} // namespace blitzdb