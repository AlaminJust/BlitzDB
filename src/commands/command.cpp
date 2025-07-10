#include "commands/command.h"
#include "core/storage/in_memory.h"
#include <memory>
#include <vector>

namespace blitzdb {

    // Base Command destructor (needed even if pure virtual)
    Command::~Command() = default;

    // Helper function to parse command arguments (Redis-like)
    std::vector<std::string> ParseCommandArgs(const std::string& resp_formatted) {
        std::vector<std::string> args;
        size_t pos = 0;

        // Simplified RESP parser (real implementation would be more robust)
        while (pos < resp_formatted.length()) {
            if (resp_formatted[pos] == '$') {
                size_t len_end = resp_formatted.find("\r\n", pos);
                int len = std::stoi(resp_formatted.substr(pos + 1, len_end - pos - 1));
                pos = len_end + 2;
                args.push_back(resp_formatted.substr(pos, len));
                pos += len + 2;
            }
            else {
                pos++;
            }
        }
        return args;
    }

    // Factory method to create commands from RESP strings
    std::unique_ptr<Command> CreateCommand(
        InMemoryStorage& storage,
        const std::string& resp_command)
    {
        auto args = ParseCommandArgs(resp_command);
        if (args.empty()) return nullptr;

        if (args[0] == "GET" && args.size() == 2) {
            return std::make_unique<GetCommand>(storage, args[1]);
        }
        else if (args[0] == "SET" && args.size() >= 3) {
            return std::make_unique<SetCommand>(storage, args[1], args[2]);
        }
        else if (args[0] == "DEL" && args.size() == 2) {
            return std::make_unique<DelCommand>(storage, args[1]);
        }

        return nullptr;  // Unsupported command
    }

} // namespace blitzdb