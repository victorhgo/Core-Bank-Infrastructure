#include "account_service.hpp"
#include "database_connection.hpp"

std::optional<Account> AccountService::getAccount(int accountID) {
    std::cout << "[AccountService] getAccount(" << accountID << ") start\n";

    auto& db = DBConnection::getInstance();
    auto guard = db.lock();
    auto tx = db.createReadTransaction();

    std::cout << "[AccountService] getAccount(" << accountID << ") before exec\n";

    pqxx::result res = tx->exec(
        "SELECT a.account_id, a.customer_id, c.full_name AS customer_name,"
        " c.email AS customer_email, a.account_type, a.balance, a.currency "
        "FROM accounts a JOIN customers c ON a.customer_id = c.customer_id "
        "WHERE a.account_id = $1", accountID
    );

    tx->commit();

    std::cout << "[AccountService] getAccount(" << accountID << ") after exec, rows = "
              << res.size() << "\n";

    if (res.empty()) {
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

    std::cout << "[AccountService] getAccount(" << accountID << ") built Account\n";

    return acc;
}


bool AccountService::accountExist(int accountID) {
    /* Check if the account exists */
    return getAccount(accountID).has_value();
}

double AccountService::getBalance(int accountId) {
    std::cout << "[AccountService] getBalance(" << accountId << ") called\n";

    auto account = getAccount(accountId);

    std::cout << "[AccountService] getBalance(" << accountId << ") after getAccount\n";

    if (!account) {
        std::cout << "[AccountService] Account " << accountId << " not found\n";
        throw std::runtime_error("Account not found: " + std::to_string(accountId));
    }

    std::cout << "[AccountService] getBalance(" << accountId << ") returning "
              << account->balance << "\n";

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