#include "commands/set_command.h"
#include <chrono>
#include <stdexcept>

namespace blitzdb {

    SetCommand::SetCommand(InMemoryStorage& storage,
        const std::string& key,
        const std::string& value,
        SetOptions options)
        : storage_(storage),
        key_(key),
        value_(value),
        options_(options) {
    }

    std::string SetCommand::execute() {
        // Handle NX (only set if key doesn't exist)
        if (options_.nx && storage_.exists(key_)) {
            return ":0\r\n";  // Redis returns 0 for failed NX
        }

        // Handle XX (only set if key exists)
        if (options_.xx && !storage_.exists(key_)) {
            return ":0\r\n";  // Redis returns 0 for failed XX
        }

        // Perform the set operation
        storage_.set(key_, value_);

        // Handle expiration if specified
        if (options_.expire_after > 0) {
            auto expire_time = std::chrono::system_clock::now() +
                std::chrono::milliseconds(options_.expire_after);
            storage_.set_expiry(key_, expire_time);
        }

        // Return success (Redis returns "+OK\r\n" for simple SET)
        if (options_.return_old_value) {
            // For GET option, return the old value or nil
            auto old_value = storage_.get(key_);
            return old_value.empty() ? "$-1\r\n" :
                "$" + std::to_string(old_value.length()) + "\r\n" + old_value + "\r\n";
        }

        return "+OK\r\n";
    }

} // namespace blitzdb