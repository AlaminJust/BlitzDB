// BlitzDB.cpp : Defines the entry point for the application.
//

#include "blitzdb.h"
#include "../src/network/server.h"
#include "asio.hpp"

using namespace std;

int main()
{
	cout << "BlitzDB Server Starting..." << endl;
    asio::io_context io_context;
    blitzdb::Server server(io_context, 6380); // Redis default port
    server.start();
    io_context.run();
	return 0;
}
