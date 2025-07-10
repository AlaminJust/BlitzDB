#pragma once
#include "command.h"
#include "../core/storage/in_memory.h"
#include <chrono>

namespace blitzdb {

    struct SetOptions {
        bool nx = false;          // Only set if key doesn't exist (NX)
        bool xx = false;          // Only set if key exists (XX)
        bool return_old_value = false; // Return old value (GET)
        long long expire_after = 0;   // Expiration in milliseconds (PX) or seconds (EX)
    };

    class SetCommand : public Command {
    public:
        SetCommand(InMemoryStorage& storage,
            const std::string& key,
            const std::string& value,
            SetOptions options = {});

        std::string execute() override;

    private:
        InMemoryStorage& storage_;
        std::string key_;
        std::string value_;
        SetOptions options_;
    };

} // namespace blitzdb