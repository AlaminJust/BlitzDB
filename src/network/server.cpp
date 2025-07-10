#include "server.h"

namespace blitzdb {

    Server::Server(asio::io_context& io_context, unsigned short port)
        : acceptor_(io_context, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port)) {
    }

    void Server::start() {
        acceptor_.async_accept([this](auto ec, auto socket) {
            if (!ec) handle_connection(std::move(socket));
            start(); // Keep accepting new connections
            });
    }

    void Server::handle_connection(asio::ip::tcp::socket socket) {
        // Parse commands and execute them
    }

} // namespace blitzdb