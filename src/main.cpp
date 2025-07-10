#include "network/server.h"
#include <asio.hpp>

int main() {
    asio::io_context io_context;
    blitzdb::Server server(io_context, 6379); // Redis default port
    server.start();
    io_context.run();
    return 0;
}