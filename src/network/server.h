#pragma once
#include <asio.hpp>
#include "../core/storage/in_memory.h"

namespace blitzdb {

    class Server {
    public:
        Server(asio::io_context& io_context, unsigned short port);
        void start();

    private:
        void handle_connection(asio::ip::tcp::socket socket);
        asio::ip::tcp::acceptor acceptor_;
        InMemoryStorage storage_;
    };

} // namespace blitzdb