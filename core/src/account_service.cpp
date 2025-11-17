#include "account_service.hpp"
#include "database_connection.hpp"

std::optional<Account> AccountService::getAccount(int accountID) {
    /* Get the instance using DBConnection and store in db */
    auto& db = DBConnection::getInstance();

    /* uses the instance to create the transaction */
    auto tx = db.createReadTransaction();

    /* Query account using createReadTransaction() interface */
    pqxx::result res = tx->exec(
        "SELECT a.account_id, a.customer_id, c.full_name AS customer_name,"
        " c.email AS customer_email, a.account_type, a.balance, a.currency "
        "FROM accounts a JOIN customers c ON a.customer_id = c.customer_id "
        "WHERE a.account_id = $1", accountID
    );

    /* Commits transaction */
    tx->commit();

    /* Check if nothing returned */
    if(res.empty()){
        return std::nullopt;
    }

    const auto& row = res[0];

    Account acc{
        row["account_id"].as<int>(),
        row["customer_id"].as<int>(),
        row["customer_name"].as<std::string>(),
        row["customer_email"].as<std::string>(),
        row["account_type"].as<std::string>(),
        row["balance"].as<double>(),
        row["currency"].as<std::string>()
    };

    return acc;
}

bool AccountService::accountExist(int accountID) {
    /* Check if the account exists */
    return getAccount(accountID).has_value();
}

double AccountService::getBalance(int accountID){
    /* fetch account using the getAccount method */
    auto account = getAccount(accountID);

    /* If account not found */
    if (!account) {
        throw std::runtime_error("Account not found: " + std::to_string(accountID));
    }

    /* Returns the balance only */
    return account->balance;
}

void AccountService::printAccount(int accountID) {
    auto openAccount = getAccount(accountID);

    /* If account not found */
    if (!openAccount) {
        throw std::runtime_error("Account not found: " + std::to_string(accountID));
    }

    /* parse info into acc */
    const auto& acc = *openAccount;

    std::cout << "Account ID:   " << acc.accountID   << "\n"
              << "Customer ID:  " << acc.customerID  << "\n"
              << "Customer Name: " << acc.customerName << "\n"
              << "Customer Email: " << acc.customerEmail << "\n"
              << "Type:         " << acc.accountType << "\n";
}