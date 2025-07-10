#include "server.h"
#include <iostream>

namespace blitzdb {

    Server::Server(asio::io_context& io_context, unsigned short port)
        : acceptor_(io_context, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port)),
        storage_() {
        std::cout << "BlitzDB 1.0 (ASIO " << ASIO_VERSION << ") listening on port " << port << std::endl;
    }

    void Server::start() {
        auto socket = std::make_shared<asio::ip::tcp::socket>(acceptor_.get_executor());

        acceptor_.async_accept(*socket, [this, socket](asio::error_code ec) {
            if (!ec) {
                std::cout << "Client connected: "
                    << socket->remote_endpoint().address().to_string()
                    << std::endl;
                handle_connection(socket);
            }
            else {
                std::cerr << "Accept error: " << ec.message() << std::endl;
            }
            start(); // Continue accepting
            });
    }

    void Server::handle_connection(std::shared_ptr<asio::ip::tcp::socket> socket) {
        auto buffer = std::make_shared<std::string>();  // Use string instead of streambuf

        asio::async_read_until(*socket, asio::dynamic_buffer(*buffer), "\r\n",
            [this, socket, buffer](const asio::error_code& ec, size_t bytes) {
                if (ec) return;

                // Extract command safely
                std::string command = buffer->substr(0, bytes);
                buffer->erase(0, bytes);  // Remove processed data

                // Ensure command lives until async_write completes
                auto response = std::make_shared<std::string>("+PONG\r\n");
                asio::async_write(*socket, asio::buffer(*response),
                    [response](const asio::error_code& ec, size_t) {  // Keep response alive
                        if (ec) std::cerr << "Write error: " << ec.message() << std::endl;
                    }
                );

                handle_connection(socket);  // Continue processing
            });
    }

    void Server::write_response(std::shared_ptr<asio::ip::tcp::socket> socket,
        const std::string& response) {
        asio::async_write(*socket, asio::buffer(response),
            [](const asio::error_code& ec, size_t) {
                if (ec) {
                    std::cerr << "Write error: " << ec.message() << std::endl;
                }
            });
    }

} // namespace blitzdb