#pragma once
#include <asio.hpp>
#include <memory>
#include <unordered_set>
#include <mutex>
#include <atomic>
#include "../core/storage/in_memory.h"

namespace blitzdb {

    // Command enumeration
    enum class Command {
        UNKNOWN,
        PING,
        SET,
        GET,
        DEL,
        QUIT,
        AUTH,
        // Add more commands here
    };

    // Command metadata structure
    struct CommandInfo {
        Command cmd;
        int min_args;
        int max_args;
        bool auth_required;
        // Add more metadata as needed (ACL categories, etc.)
    };

    class Server {
    public:
        Server(asio::io_context& io_context, unsigned short port);

        // Start accepting connections
        void start();

        // Shutdown gracefully
        void stop();

    private:
        // Connection handlers
        void handle_connection(std::shared_ptr<asio::ip::tcp::socket> socket);
        void handle_disconnection(std::shared_ptr<asio::ip::tcp::socket> socket);

        // Command processing
        std::vector<std::string> parse_command(const std::string& input);
        bool validate_command(const CommandInfo& info, const std::vector<std::string>& tokens);
        std::string process_command(std::shared_ptr<asio::ip::tcp::socket> socket,
            const std::vector<std::string>& tokens);

        // Response handling
        void write_response(
            std::shared_ptr<asio::ip::tcp::socket> socket,
            const std::string& response,
            std::function<void()> callback = nullptr
        );

        // Authentication
        bool is_authenticated(std::shared_ptr<asio::ip::tcp::socket> socket);
        void authenticate_client(std::shared_ptr<asio::ip::tcp::socket> socket);
        void deauthenticate_client(std::shared_ptr<asio::ip::tcp::socket> socket);

        // Members
        asio::ip::tcp::acceptor acceptor_;
        InMemoryStorage storage_;
        std::atomic<bool> running_{ false };

        // Connection tracking
        std::unordered_set<std::shared_ptr<asio::ip::tcp::socket>> authenticated_clients_;
        std::vector<std::weak_ptr<asio::ip::tcp::socket>> active_connections_;
        std::mutex connections_mutex_;
        std::mutex auth_mutex_;

        // Command mappings (could be moved to a separate class)
        static const std::unordered_map<std::string, Command> command_map;
        static const std::unordered_map<Command, CommandInfo> command_info;
    };

} // namespace blitzdb