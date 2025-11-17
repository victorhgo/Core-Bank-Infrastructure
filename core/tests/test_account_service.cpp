/* These integration tests for AccountService using GoogleTest assumes:
 *
 *  - DB is running
 *  - json credential is valid
 *  - accounts and customers tables exist
 *  - at least one account row exists */

#include <gtest/gtest.h>
#include "account_service.hpp"
#include "database_connection.hpp"

/* Existing account */
short EXISTING_ACCOUNT_ID = 1;
/* Non existing account */
int NON_EXISTING_ACCOUNT_ID = 99999;

/**
 * @brief GoogleTest tool for integration tests of the ServiceAccount class.
 *
 * We need to ensure the DB connection is initialized once before each test
 * Tests can safely use DBConnection and AccountService without repeating
 * connection setup logic.
 */
class AccountServiceTest : 

public ::testing::Test {
    
    protected:
        void SetUp() override {
            auto& db = DBConnection::getInstance();

            if (!db.isConnected()) {
                db.loadConfig("config/db_credential.json");
                db.connect();
            }
        }

AccountService service;
};

/**
 * @brief Existing account should be returned as a populated std::optional
 */
TEST_F(AccountServiceTest, GetAccount_ReturnsAccountForExistingId) {
    auto accOpt = service.getAccount(EXISTING_ACCOUNT_ID);

    ASSERT_TRUE(accOpt.has_value()) 
        << "Expected account with id = " << EXISTING_ACCOUNT_ID << " to exist";

    const auto& acc = *accOpt;
    EXPECT_EQ(acc.accountID, EXISTING_ACCOUNT_ID);
    EXPECT_FALSE(acc.currency.empty());
    EXPECT_FALSE(acc.accountType.empty());
}

/**
 * @brief Non-existing account should return std::nullopt.
 */
TEST_F(AccountServiceTest, GetAccount_ReturnsNulloptForNonExistingId) {
    auto accOpt = service.getAccount(NON_EXISTING_ACCOUNT_ID);
    EXPECT_FALSE(accOpt.has_value());
}

/**
 * @brief accountExist() should return true for existing accounts
 */
TEST_F(AccountServiceTest, AccountExist_ReturnsTrueForExistingAccount) {
    EXPECT_TRUE(service.accountExist(EXISTING_ACCOUNT_ID));
}

/**
 * @brief accountExist() should return false for non existing accounts
 */
TEST_F(AccountServiceTest, AccountExist_ReturnsFalseForNonExistingAccount) {
    EXPECT_FALSE(service.accountExist(NON_EXISTING_ACCOUNT_ID));
}

/**
 * @brief getBalance() should return a non-negative balance for the existing account.
 *
 */
TEST_F(AccountServiceTest, GetBalance_ReturnsBalanceForExistingAccount) {
    double balance = 0.0;

    EXPECT_NO_THROW({
        balance = service.getBalance(EXISTING_ACCOUNT_ID);
    });

    /* A basic sanity check where in a normal bank account, balance is rarely negative (but can happen in this one)
     *  Adjust/remove this if your seed data intentionally has negative balances. */
    EXPECT_GE(balance, 0.0);
}

/**
 * @brief getBalance() should throw when called for a non-existing account.
 */
TEST_F(AccountServiceTest, GetBalance_ThrowsForNonExistingAccount) {
    EXPECT_THROW(
        service.getBalance(NON_EXISTING_ACCOUNT_ID),
        std::runtime_error
    );
}

/**
 * @brief printAccount() should not throw exception for an existing account.
 *
 * We don't assert on output here, just that it runs successfully.
 */
TEST_F(AccountServiceTest, PrintAccount_DoesNotThrowForExistingAccount) {
    EXPECT_NO_THROW({
        service.printAccount(EXISTING_ACCOUNT_ID);
    });
}

/**
 * @brief printAccount() should throw exception for a non existing account.
 *
 * In your current implementation, it prints a "not found" message and returns.
 */
TEST_F(AccountServiceTest, PrintAccount_ThrowsForNonExistingAccount) {
    EXPECT_THROW(
        service.printAccount(NON_EXISTING_ACCOUNT_ID),
        std::runtime_error
    );
}
