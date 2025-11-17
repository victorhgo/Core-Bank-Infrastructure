/* Transaction Service header, declare the money transfer method */
#ifndef TRANSACTIONS_HPP
#define TRANSACTIONS_HPP

/* Includes */
#include <iostream>
/* libpqxx */
#include <pqxx/pqxx>
/* Handles string */
#include <string>
#include <optional>
/* Deals with exceptions */
#include <stdexcept>

/**
 * @brief Service class responsible for performing money transfers
 *        between accounts using the database stored procedure.
 *
 * TransactionService acts as a thin wrapper around transferMoney() stored procedure
 *
 */
class TransactionService {

public:
    /**
     * @brief Perform a transaction between fromAccount to toAccount
     * 
     * This function starts a write transaction using DBConnection method then
     * calls the transferMoney(from, to, amount, description) stored procedure
     * from DB then commits the transaction if successful. If not throws
     * std::runtime_error if any failure reported by the database or by 
     * connection or transaction services
     *
     * @param fromAccountID source account unique id number
     * @param toAccountID destiny account unique id number
     * @param amount double qnt of money to be sent
     * @param description optional string, describes transfer
     *
     */
    void transfer(int fromAccountID,
         int toAccountID,
         double amount,
         const std::string& description = "Transfer description...");
};

#endif