#include "transactions.hpp"
#include "database_connection.hpp"

void TransactionService::transfer(int fromAccountID,
                                  int toAccountID,
                                  double amount,
                                  const std::string& description) {
    
    /* Get the instance using DBConnection and store in db */
    auto& db = DBConnection::getInstance();

    /* uses the instance to create a write transaction */
    auto tx = db.createWriteTransaction();

    try{
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

        // Commit if everything succeeded
        tx->commit();
    }
    /* Throws exception for failed transfers */
    catch (const std::exception& e){
        std::string msg = "Transfer failed: ";
        msg += e.what();
        throw std::runtime_error(msg);
    }
}