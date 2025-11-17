/* Here we will declare our account service module class and
basic functionalities */
#ifndef ACCOUNT_SERVICE_HPP
#define ACCOUNT_SERVICE_HPP

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
 * @brief structure to store account to be fetched from getAccount()
 * 
 */
struct Account {
    int         accountID;
    int         customerID;
    std::string customerName;
    std::string customerEmail;
    std::string accountType;
    double      balance;
    std::string currency;
};

/**
 * @class Service class that provides high level operations for bank accounts 
 * 
 * This class will use DBConnection to query the database and exposes a simple
 *  interface to access data from db.
 * 
 */
class AccountService{
    public:
        
        /**
         * @brief Get the Account object by unique accountID
         * 
         * @param accountID integer, unique account identification 
         * @return Account struct containing the account data if found,
         * or std::nullopt if no account exists with the given ID */
        std::optional<Account> getAccount(int accountID);

        /**
         * @brief Check if account exists
         * 
         * @param accountID integer
         * @return true if account is found in database
         * @return false otherwise
         */
        bool accountExist(int accountID);

        /**
         * @brief Get the Account Balance object
         * 
         * @param accountID 
         * @return double as account balance
         */
        double getBalance(int accountID);

        /**
         * @brief Prints account information
         * 
         * @param accountID 
         */
        void printAccount(int accountID);
};

#endif