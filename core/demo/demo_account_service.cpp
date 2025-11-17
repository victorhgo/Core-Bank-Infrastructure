/* Demonstration to test the account service implementation, we can make queries to the database
by parsing the account number with the exec command. This service is dependant from database_connection*/
#include "database_connection.hpp"
#include "account_service.hpp"

int main(int argc, char* argv[]) {
    try {
        std::cout << "=== AccountService Demonstration ===\n\n";

        /* To initialize the connection to db thru DBConnection */
        auto& db = DBConnection::getInstance();

        std::cout << "[INFO] Loading DB config...\n";
        /* Load config from json */
        db.loadConfig("config/db_credential.json");

        /* Connect to database */
        std::cout << "[INFO] Connecting to PostgreSQL...\n";
        db.connect();

        /* Check the connection */
        if (!db.isConnected()) {
            std::cerr << "[ERROR] Failed to connect to database.\n";
            return 1;
        }

        std::cout << "[OK] Connected to database.\n\n";

        /* Fetchs the account id from command line (default = 1)*/
        int accountID = 1;

        if (argc > 1) {
            /* Check if account entered is valid, integer */
            try {
                accountID = std::stoi(argv[1]);
            } catch (const std::exception&) {
                std::cerr << "[WARN] Invalid account ID argument. Using default: 1\n";
            }
        }

        /* Display which accountID was selected */
        std::cout << "[INFO] Querying account_id = " << accountID << "...\n";

        /* Using AccountService to fetch and print account info */
        AccountService service;

        /* First we check if the account exists */
        if (!service.accountExist(accountID)) {
            std::cout << "Account " << accountID << " does not exist.\n";
            return 0;
        }

        /* We can print some information from printAccount() */
        service.printAccount(accountID);

        /* Uses the getBalance() module to fetch the account's balance */
        double balance = service.getBalance(accountID);
        std::cout << "\n[INFO] Querying balance for account_id = " << accountID;
        std::cout << "\nBalance: " << balance << "\n";
        std::cout << "\n=== Demo finished successfully ===\n";
    }
    /* If conenction can't be made */
    catch (const std::exception& e) {
        std::cerr << "[FATAL] Exception: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
