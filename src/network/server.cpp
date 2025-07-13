#include "server.h"
#include <iostream>
#include <algorithm>
#include <sstream>
#include <unordered_map>
#include <unordered_set>

namespace blitzdb {

    const std::unordered_map<std::string, Command> Server::command_map = {
        {"PING", Command::PING},
        {"SET", Command::SET},
        {"GET", Command::GET},
        {"DEL", Command::DEL},
        {"QUIT", Command::QUIT},
        {"AUTH", Command::AUTH},
    };

    const std::unordered_map<Command, CommandInfo> Server::command_info = {
        {Command::PING, {Command::PING, 0, 0, false}},
        {Command::SET, {Command::SET, 2, 2, false}},
        {Command::GET, {Command::GET, 1, 1, false}},
        {Command::DEL, {Command::DEL, 1, -1, true}},
        {Command::QUIT, {Command::QUIT, 0, 0, false}},
        {Command::AUTH, {Command::AUTH, 1, 1, false}},
    };

    Server::Server(asio::io_context& io_context, unsigned short port)
        : acceptor_(io_context, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port)),
        storage_(), running_(true) {
        std::cout << "BlitzDB server listening on port " << port << std::endl;
    }

    void Server::start() {
        if (!running_.load()) return;

        auto socket = std::make_shared<asio::ip::tcp::socket>(acceptor_.get_executor());

        acceptor_.async_accept(*socket, [this, socket](asio::error_code ec) {
            if (!ec) {
                {
                    std::lock_guard<std::mutex> lock(connections_mutex_);
                    if (!running_.load()) return;

                    active_connections_.emplace_back(socket);

                    std::cout << "New connection from: "
                        << socket->remote_endpoint().address().to_string()
                        << std::endl;
                }

                handle_connection(socket);
            }
            else if (ec != asio::error::operation_aborted) {
                std::cerr << "Accept error: " << ec.message() << std::endl;
            }

            if (running_.load()) {
                start();
            }
            });
    }

    void Server::stop() {
        running_ = false;

        std::lock_guard<std::mutex> lock(connections_mutex_);
        for (auto& weak_conn : active_connections_) {
            if (auto conn = weak_conn.lock()) {
                asio::error_code ec;
                conn->shutdown(asio::ip::tcp::socket::shutdown_both, ec);
                conn->close(ec);
            }
        }
        active_connections_.clear();

        asio::error_code ec;
        acceptor_.close(ec);
    }

    void Server::handle_connection(std::shared_ptr<asio::ip::tcp::socket> socket) {
        if (!socket || !socket->is_open()) {
            std::cerr << "Invalid socket in handle_connection" << std::endl;
            return;
        }

        auto buffer = std::make_shared<std::string>();  // Use string as buffer

        asio::async_read_until(*socket, asio::dynamic_buffer(*buffer), "\r\n",
            [this, socket, buffer](const asio::error_code& ec, size_t bytes) {
                if (ec) {
                    if (ec != asio::error::eof && ec != asio::error::operation_aborted) {
                        std::cerr << "Read error: " << ec.message() << std::endl;
                    }
                    handle_disconnection(socket);
                    return;
                }

                try {
                    // Extract the line (including the delimiter)
                    std::string line = buffer->substr(0, bytes);
                    buffer->erase(0, bytes);  // Remove processed data

                    // Clean CRLF
                    if (!line.empty() && line.back() == '\n') {
                        line.pop_back();
                        if (!line.empty() && line.back() == '\r') {
                            line.pop_back();
                        }
                    }

                    // Process command if not empty
                    if (!line.empty()) {
                        auto tokens = parse_command(line);
                        auto response = std::make_shared<std::string>(process_command(socket, tokens));

                        if (!tokens.empty() && tokens[0] == "QUIT") {
                            asio::async_write(*socket, asio::buffer(*response),
                                [this, socket](const asio::error_code&, size_t) {
                                    handle_disconnection(socket);
                                });
                            return;
                        }

                        asio::async_write(*socket, asio::buffer(*response),
                            [this, socket, response](const asio::error_code& ec, size_t) {
                                if (ec) {
                                    std::cerr << "Write error: " << ec.message() << std::endl;
                                    handle_disconnection(socket);
                                    return;
                                }
                                handle_connection(socket);  // Continue processing
                            });
                    }
                    else {
                        handle_connection(socket);  // No data, continue
                    }
                }
                catch (const std::exception& e) {
                    std::cerr << "Processing error: " << e.what() << std::endl;
                    handle_disconnection(socket);
                }
            });
    }

    void Server::handle_disconnection(std::shared_ptr<asio::ip::tcp::socket> socket) {
        deauthenticate_client(socket);

        std::lock_guard<std::mutex> lock(connections_mutex_);
        active_connections_.erase(
            std::remove_if(active_connections_.begin(), active_connections_.end(),
                [socket](const std::weak_ptr<asio::ip::tcp::socket>& weak_conn) {
                    auto conn = weak_conn.lock();
                    return !conn || conn == socket;
                }),
            active_connections_.end()
        );
    }

    std::vector<std::string> Server::parse_command(const std::string& input) {
        std::vector<std::string> tokens;
        size_t start = 0;
        size_t end = input.find(' ');

        while (end != std::string::npos) {
            std::string token = input.substr(start, end - start);
            if (!token.empty()) {
                tokens.push_back(token);
            }
            start = end + 1;
            end = input.find(' ', start);
        }

        // Add the last token
        std::string last_token = input.substr(start);
        if (!last_token.empty()) {
            tokens.push_back(last_token);
        }

        // Clean any remaining CR/LF from tokens
        for (auto& token : tokens) {
            token.erase(std::remove(token.begin(), token.end(), '\r'), token.end());
            token.erase(std::remove(token.begin(), token.end(), '\n'), token.end());
        }

        return tokens;
    }

    bool Server::validate_command(const CommandInfo& info, const std::vector<std::string>& tokens) {
        size_t arg_count = tokens.size() - 1;
        if (info.min_args >= 0 && arg_count < static_cast<size_t>(info.min_args)) {
            return false;
        }
        if (info.max_args >= 0 && arg_count > static_cast<size_t>(info.max_args)) {
            return false;
        }
        return true;
    }

    std::string Server::process_command(std::shared_ptr<asio::ip::tcp::socket> socket,
        const std::vector<std::string>& tokens) {
        
        
        std::cout << "=== DEBUG START ===" << std::endl;
        std::cout << "Received " << tokens.size() << " tokens" << std::endl;

        if (tokens.empty()) {
            std::cout << "DEBUG: No tokens provided" << std::endl;
            return "-ERR no command provided\r\n";
        }

        // Print each token with index and length
        for (size_t i = 0; i < tokens.size(); ++i) {
            std::cout << "Token " << i << ": [" << tokens[i] << "] (length: "
                << tokens[i].length() << ")" << std::endl;
        }

        if (tokens.empty()) {
            return "-ERR no command provided\r\n";
        }


        std::string cmd_str = tokens[0];
        std::transform(cmd_str.begin(), cmd_str.end(), cmd_str.begin(), ::toupper);

        auto cmd_it = command_map.find(cmd_str);
        if (cmd_it == command_map.end()) {
            return "-ERR unknown command '" + cmd_str + "'\r\n";
        }

        Command cmd = cmd_it->second;
        const CommandInfo& info = command_info.at(cmd);

        if (!validate_command(info, tokens)) {
            return "-ERR wrong number of arguments for '" + cmd_str + "' command\r\n";
        }

        if (info.auth_required && !is_authenticated(socket)) {
            return "-NOAUTH Authentication required\r\n";
        }

        switch (cmd) {
        case Command::PING:
            return "+PONG\r\n";

        case Command::SET:
            storage_.set(tokens[1], tokens[2]);
            return "+OK\r\n";

        case Command::GET: {
            std::string value = storage_.get(tokens[1]);
            if (value.empty()) {
                return "$-1\r\n";
            }
            return "$" + std::to_string(value.size()) + "\r\n" + value + "\r\n";
        }

        case Command::DEL: {
            int deleted_count = 0;
            for (size_t i = 1; i < tokens.size(); ++i) {
                if (storage_.del(tokens[i])) {
                    deleted_count++;
                }
            }
            return ":" + std::to_string(deleted_count) + "\r\n";
        }

        case Command::QUIT:
            return "+OK\r\n";

        case Command::AUTH:
            if (tokens[1] == "defaultpass") {
                authenticate_client(socket);
                return "+OK\r\n";
            }
            return "-ERR invalid password\r\n";

        default:
            return "-ERR unknown command\r\n";
        }
    }

    void Server::write_response(std::shared_ptr<asio::ip::tcp::socket> socket,
        const std::string& response,
        std::function<void()> callback) {
        asio::async_write(*socket, asio::buffer(response),
            [callback](const asio::error_code& ec, size_t) {
                if (ec) {
                    std::cerr << "Write error: " << ec.message() << std::endl;
                    return;
                }
                if (callback) {
                    callback();
                }
            });
    }

    bool Server::is_authenticated(std::shared_ptr<asio::ip::tcp::socket> socket) {
        std::lock_guard<std::mutex> lock(auth_mutex_);
        return authenticated_clients_.find(socket) != authenticated_clients_.end();
    }

    void Server::authenticate_client(std::shared_ptr<asio::ip::tcp::socket> socket) {
        std::lock_guard<std::mutex> lock(auth_mutex_);
        authenticated_clients_.insert(socket);
    }

    void Server::deauthenticate_client(std::shared_ptr<asio::ip::tcp::socket> socket) {
        std::lock_guard<std::mutex> lock(auth_mutex_);
        authenticated_clients_.erase(socket);
    }

} // namespace blitzdb
