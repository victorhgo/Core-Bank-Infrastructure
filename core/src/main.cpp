/* Main file for the running transaction Server */
#include "database_connection.hpp"
#include "server.hpp"
#include <iostream>
#include <string>
#include <cstdlib>

/**
 * @brief Simple helper to parse host/port from argv.
 *
 * Usage:
 *   ./server : default host = 0.0.0.0, port = 5555
 *   ./server 127.0.0.1 6000 : host = 127.0.0.1, port = 6000
 */
static void parseArgs(int argc, char* argv[],
                      std::string& hostOut,
                      int& portOut) {
    hostOut = "0.0.0.0";
    portOut = 8080;

    if (argc >= 2) {
        hostOut = argv[1];
    }
    if (argc >= 3) {
        portOut = std::atoi(argv[2]);
        if (portOut <= 0 || portOut > 65535) {
            throw std::runtime_error("Invalid port number: " + std::string(argv[2]));
        }
    }
}

int main(int argc, char* argv[]) {
    try {
        std::string host;
        int port;
        parseArgs(argc, argv, host, port);

        std::cout << "[Main] Starting Transaction Server...\n";
        std::cout << "[Main] Using host = " << host
                  << ", port = " << port << "\n";

        /* Init and connect the DB */
        auto& db = DBConnection::getInstance();
        db.loadConfig("config/db_credential.json");
        db.connect();

        if (!db.isConnected()) {
            std::cerr << "[Main] Failed to connect to the database.\n";
            return 1;
        }

        std::cout << "[Main] Connected to database successfully.\n";

        /* Starts the TCP server on host,port */
        Server server(host, port);
        server.start();

        std::cout << "[Main] Server running on " << host << ":" << port
                  << ". Press Enter to stop...\n";

        /* Simple shutdown mechanism for tests,
         * wait for user to press Enter. */
        std::cin.get();

        std::cout << "[Main] Shutting down server...\n";
        server.stop();
        std::cout << "[Main] Server stopped cleanly.\n";
    }
    catch (const std::exception& e) {
        std::cerr << "[FATAL] " << e.what() << "\n";
        return 1;
    }

    return 0;
}