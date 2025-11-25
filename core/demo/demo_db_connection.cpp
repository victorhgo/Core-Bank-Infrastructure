/* Demo program to test the database connection and perform a simple query
 * Of course a test suit should be implemented to validate the class and all functions
 * but for now, this will suffice.
 *
 * This code was adapted from the hello.cpp example provided by the libpqxx */
#include "database_connection.hpp"
#include "json.hpp"

int main() {
    try {
        /* Load configuration from json */
        std::cout << "Loading credential from JSON file..." << std::endl;
        DBConnection::getInstance().loadConfig("config/db_credential.json");

        /* Connect to database */
        std::cout << "Connecting to PostgreSQL..." << std::endl;
        DBConnection::getInstance().connect();

        if (DBConnection::getInstance().isConnected()) {
            std::cout << "Connected to PostgreSQL!" << std::endl;
        }

        /* Test a simple query on the database */
        std::cout << "Running test query...\n" << std::endl;
        auto tx = DBConnection::getInstance().createReadTransaction();

        pqxx::result res = tx->exec("SELECT current_database(), now()");
        pqxx::row row = res.one_row();

        std::string dbname = row[0].as<std::string>();
        std::string now = row[1].as<std::string>();

        std::cout << "Connected to database: " << dbname << std::endl;
        std::cout << "Current server time: " << now << std::endl;

        /* Try to query a customer list */
        std::cout << "\n-- Testing a query. Return some details of customer_id = 2 --" << std::endl;
        pqxx::result cust = tx->exec("SELECT * FROM customers WHERE customer_id = 2;");
        pqxx::row row_customer = cust.one_row();

        std::string custName = row_customer[1].as<std::string>();
        std::string custEmail = row_customer[2].as<std::string>();
        std::string custPhone = row_customer[3].as<std::string>();
        std::string custAddress = row_customer[5].as<std::string>();

        std::cout << "Customer Name: " << custName << std::endl;
        std::cout << "Customer Email: " << custEmail << std::endl;
        std::cout << "Customer Phone: " << custPhone << std::endl;
        std::cout << "Customer Address: " << custAddress << std::endl;
        /* It's working */

        tx->commit();
    }
    catch (const std::exception &e) {
        std::cerr << "[ERROR] " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
