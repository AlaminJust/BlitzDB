#pragma once
#include <asio.hpp>
#include <memory>
#include "../core/storage/in_memory.h"

namespace blitzdb {

    class Server {
    public:
        Server(asio::io_context& io_context, unsigned short port);

        // Start accepting connections
        void start();

        // Shutdown gracefully
        void stop();

    private:
        // Enhanced connection handler with RESP support
        void handle_connection(std::shared_ptr<asio::ip::tcp::socket> socket);

        // Send Redis protocol responses
        void write_response(
            std::shared_ptr<asio::ip::tcp::socket> socket,
            const std::string& response
        );

        // Members
        asio::ip::tcp::acceptor acceptor_;
        InMemoryStorage storage_;
        std::atomic<bool> running_{ false };

        // Connection tracking
        std::vector<std::weak_ptr<asio::ip::tcp::socket>> active_connections_;
        std::mutex connections_mutex_;
    };

} // namespace blitzdb