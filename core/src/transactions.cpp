#include "transactions.hpp"
#include "database_connection.hpp"

void TransactionService::transfer(int fromAccountID,
                                  int toAccountID,
                                  double amount,
                                  const std::string& description) {
    

    std::cout << "[TransactionService] transfer("
              << fromAccountID << " -> " << toAccountID
              << ", " << amount << ", \"" << description << "\") start\n";

    /* Get the instance using DBConnection and store in db */
    auto& db = DBConnection::getInstance();
    
    /* guarantees exclusive DB usage in this scope */
    auto guard = db.lock();
    
    /* uses the instance to create a write transaction */
    auto tx = db.createWriteTransaction();

    try{

        std::cout << "[TransactionService] Calling transferMoney() in DB\n";

        pqxx::params parameters;
        parameters.append(fromAccountID);   // $1
        parameters.append(toAccountID);     // $2
        parameters.append(amount);          // $3
        parameters.append(description);     // $4

        // Call the stored procedure transferMoney(from, to, amount, description)
        tx->exec(
            "SELECT transferMoney($1, $2, $3, $4);",
            parameters
        );

        std::cout << "[TransactionService] transferMoney() executed, committing\n";
        /* Commits if everything is successfull */
        tx->commit();
        std::cout << "[TransactionService] transfer() committed successfully\n";
    }
    /* Throws exception for failed transfers */
    catch (const std::exception& e){
        std::cout << "[TransactionService] transfer() error: " << e.what() << "\n";
        throw std::runtime_error(std::string("Transfer failed: ") + e.what());
    }
}