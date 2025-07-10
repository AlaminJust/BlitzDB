#include "commands/get_command.h"
#include "../core/storage/in_memory.h"

namespace blitzdb {

    GetCommand::GetCommand(InMemoryStorage& storage, const std::string& key)
        : storage_(storage), key_(key) {
    }

    std::string GetCommand::execute() {
        // Redis-style response formatting
        const std::string& value = storage_.get(key_);

        if (value.empty()) {
            // Return nil bulk string (Redis protocol format: "$-1\r\n")
            return "$-1\r\n";
        }

        // Return bulk string (Redis protocol format: "$<length>\r\n<data>\r\n")
        return "$" + std::to_string(value.length()) + "\r\n" + value + "\r\n";
    }

} // namespace blitzdb