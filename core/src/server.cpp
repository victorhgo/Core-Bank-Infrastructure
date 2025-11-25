/* Headers for server */
#include "server.hpp"
#include "database_connection.hpp"
#include "account_service.hpp"
#include "transactions.hpp"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <iostream>
#include <sstream>
#include <cstring>


Server::Server(const std::string& host, int port) : hostBind(host), portBind(port) {

}

Server::~Server() {
    stop();
}

void Server::start() {
    /* If server is currently running, returns immediately */
    if (running) {
        return;
    }

    listenSocket = ::socket(AF_INET, SOCK_STREAM,0);
    running = true;

    if (listenSocket < 0) {
        throw std::runtime_error("Failed to create socket!");
    }

    int option = 1;

    setsockopt(listenSocket, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY); // binds to all interfaces
    addr.sin_port = htons(portBind);

    if (bind(listenSocket, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
        ::close(listenSocket);
        throw std::runtime_error("Failed to bind socket");
    }

    if (listen(listenSocket, 8) < 0) {
        ::close(listenSocket);
        throw std::runtime_error("Failed to listen on socket");
    }

    std::cout << "[Server] Listening on port " << portBind << std::endl;

    /* Accept loop in its own thread */
    acceptThread = std::thread(&Server::acceptLoop, this);

}

void Server::stop() {
    if (!running) {
        return;
    }

    running = false;

    if (listenSocket >= 0) {
        ::shutdown(listenSocket, SHUT_RDWR);
        ::close(listenSocket);
        listenSocket = -1;
    }

    if (acceptThread.joinable()) {
        acceptThread.join();
    }
}

void Server::acceptLoop() {
    while (running) {
        int clientSocket = ::accept(listenSocket, nullptr, nullptr);

        if (clientSocket < 0) {
            if (running) {
                std::perror("[Server] accept");
            }
            continue;
        }

        std::thread t(&Server::handleClient, this, clientSocket);
        /* each client is handled in its own thread */
        t.detach();
    }
}

void Server::handleClient(int clientSocket) {
    AccountService accountService;
    TransactionService txService;

    char buffer[1024];

    while (true) {
        std::memset(buffer, 0, sizeof(buffer));
        ssize_t n = ::recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        if (n <= 0) {
            /* When  client is closed or error */
            std::cout << "[Server] recv returned " << n << ", closing client\n";
            break;
        }

        /* Builds string from the exact number of bytes received */
        std::string line(buffer, n);

        while (!line.empty() && (line.back() == '\n' || line.back() == '\r')) {
            line.pop_back();
        }

        if (line.empty()) {
            std::cout << "[Server] Received empty line, ignoring\n";
            continue;
        }

        std::cout << "[Server] Received: \"" << line << "\"\n";

        std::istringstream iss(line);
        std::string cmd;
        iss >> cmd;

        std::ostringstream response;

        try {
            if (cmd == "PING") {
                std::cout << "[Server] Handling PING\n";
                response << "PONG\n";
            } else if (cmd == "BALANCE") {
                int accId;
                iss >> accId;
                if (!iss) {
                    std::cout << "[Server] BALANCE: invalid arguments\n";
                    response << "ERROR Invalid BALANCE arguments\n";
                } else {
                    std::cout << "[Server] BALANCE for account " << accId << "\n";

                    double bal = accountService.getBalance(accId);
                    response << "BALANCE " << accId << " " << bal << "\n";
                }
            } else if (cmd == "TRANSFER") {
                int fromId, toId;
                double amount;
                iss >> fromId >> toId >> amount;
                if (!iss) {
                    std::cout << "[Server] TRANSFER: invalid arguments\n";
                    response << "ERROR Invalid TRANSFER arguments\n";
                } else {

                    std::cout << "[Server] TRANSFER reqyest " << amount
                              << " from " << fromId << " to " << toId << "\n";
                    
                    try {
                        txService.transfer(fromId, toId, amount, "Server transfer");
                        std::cout << "[Server] TRANSFER succeeded\n";
                        response << "OK\n";
                    } catch (const std::exception& e) {
                        std::cout << "[Server] TRANSFER exception: " << e.what() << "\n";
                        response << "ERROR " << e.what() << "\n";
                    }
                }
            } else {
                std::cout << "[Server] Unknown command: " << cmd << "\n";
                response << "ERROR Unknown command\n";
            }
        }
        catch (const std::exception& e) {
            std::cout << "[Server] Exception: " << e.what() << "\n";
            response << "ERROR " << e.what() << "\n";
        }

        const std::string out = response.str();
        if (!out.empty()) {
            ::send(clientSocket, out.c_str(), out.size(), 0);
            std::cout << "[Server] Sent: \"" << out << "\"\n";
        } else {
            std::cout << "[Server] No response generated for: \"" << line << "\"\n";
        }
    }

    ::close(clientSocket);
    std::cout << "[Server] Client disconnected\n";
}
