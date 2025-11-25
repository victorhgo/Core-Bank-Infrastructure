#include "database_connection.hpp"
#include "server.hpp"

#include <iostream>

int main() {
    try {
        auto& db = DBConnection::getInstance();
        db.loadConfig("config/db_credential.json");
        db.connect();

        if (!db.isConnected()) {
            std::cerr << "Failed to connect to DB\n";
            return 1;
        }

        Server server("0.0.0.0", 8080);
        server.start();

        std::cout << "Server running on port 8080. Press Enter to stop...\n";
        std::cin.get();

        server.stop();
    } catch (const std::exception& e) {
        std::cerr << "[FATAL] " << e.what() << "\n";
        return 1;
    }

    return 0;
}
