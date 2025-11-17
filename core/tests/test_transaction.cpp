/* Here we will implement some Integration tests for TransactionService using GoogleTest
 * These integration tests for AccountService using GoogleTest assumes:
 *
 *  - DB is running
 *  - json credential is valid
 *  - accounts and customers tables exist
 *  - at least one account row exists
 *  - transferMoney() SQL stored procedure defined in db */
#include <gtest/gtest.h>
#include "database_connection.hpp"
#include "account_service.hpp"
#include "transactions.hpp"

short FROM_USD_ACCOUNT_ID = 1;
short TO_USD_ACCOUNT_ID = 2;

/* Create a EUR account for testing the restriction on different currency transfers */
int EUR_ACCOUNT_ID = 9001;

/**
 * @class TransactionServiceTest
 * 
 * @brief GoogleTest tool for integration tests of the TransactionService
 *
* We need to ensure the DB connection is initialized once before each test
 * Tests can safely use DBConnection and TransactionService without repeating
 * connection setup logic.
 * We also need to ensure for this test that FROM_USD_ACCOUNT_ID and 
 * TO_USD_ACCOUNT_ID exist and EUR_ACCOUNT_ID exists and has currency set to 'EUR'
 */
class TransactionServiceTest : 

public ::testing::Test {
    
    protected:

        void SetUp() override {
            auto& db = DBConnection::getInstance();

            if (!db.isConnected()) {
                db.loadConfig("config/db_credential.json");
                db.connect();
            }

            /* Sanity check to that both USD account exists */
            ASSERT_TRUE(accountService.accountExist(FROM_USD_ACCOUNT_ID))
                << "FROM_USD_ACCOUNT_ID (" << FROM_USD_ACCOUNT_ID << ") does not exist. "
                << "Adjust the ID or seed your DB accordingly.";
            ASSERT_TRUE(accountService.accountExist(TO_USD_ACCOUNT_ID))
                << "TO_USD_ACCOUNT_ID (" << TO_USD_ACCOUNT_ID << ") does not exist.";

            /* Do we have an euro account? If not, create it function */
            ensureEurTestAccount();
        }

        /**
        * @brief This function tests if euro account exists, if not
        * creates it
        * 
        */
        void ensureEurTestAccount() {
            auto& db = DBConnection::getInstance();

            auto account = accountService.getAccount(EUR_ACCOUNT_ID);
            if (account) {
                /* If account exists but it's not euro Currency, throws error */
                ASSERT_EQ(account->currency, "EUR")
                    << "EUR_ACCOUNT_ID (" << EUR_ACCOUNT_ID
                    << ") exists but currency is " << account->currency
                    << ", expected EUR.";
                return;
            }

            /* creates a simple EUR account for tests also assumes that account 1 exists */
            auto tx = db.createWriteTransaction();
            tx->exec(
                "INSERT INTO accounts (account_id, customer_id, account_type, balance, currency) "
                "VALUES (9001, 1, 'checking', 100.00, 'EUR');"
            );
            /* Commits transaction */
            tx->commit();

            /* Asserts the new account exists and the currency is EUR */
            auto eurAcc = accountService.getAccount(EUR_ACCOUNT_ID);
            ASSERT_TRUE(eurAcc.has_value());
            ASSERT_EQ(eurAcc->currency, "EUR");
        }

        /* Account esrvice and transactionService */
        AccountService accountService;
        TransactionService transactionService;
};

/**
 * @brief Helper to get balances of two accounts at once using accountService module
 */
static std::pair<double, double> getBalances(AccountService& svc,
                                             int fromId,
                                             int toId) {
    double fromBal = svc.getBalance(fromId);
    double toBal   = svc.getBalance(toId);
    return {fromBal, toBal};
}

/**
 * @brief Transfer should succeed for valid accounts and positive amount,
 * and adjust balances accordingly
 */
TEST_F(TransactionServiceTest, Transfer_SucceedsForValidAccountsAndAmount) {
    const double amount = 10.0;

    /* Asserts the current balance */
    auto [fromBefore, toBefore] = getBalances(accountService,
                                              FROM_USD_ACCOUNT_ID,
                                              TO_USD_ACCOUNT_ID);

    /* Throws error if balance is not enought for test */
    ASSERT_GE(fromBefore, amount)
        << "Source account does not have enough balance for this test.";

    /* Executes the transference */
    EXPECT_NO_THROW({
        transactionService.transfer(FROM_USD_ACCOUNT_ID, TO_USD_ACCOUNT_ID, amount,
                           "Test transfer - success case");
    });

    /* Check if the balance is different now */
    auto [fromAfter, toAfter] = getBalances(accountService,
                                            FROM_USD_ACCOUNT_ID,
                                            TO_USD_ACCOUNT_ID);
    
    /* Check the balance remain unchanged
     * 0.000001 = 1e-6, tolerance for floating-point comparison
     * how much difference acceptable to still say the values are equal */
    EXPECT_NEAR(fromAfter, fromBefore - amount, 1e-6);
    EXPECT_NEAR(toAfter,   toBefore   + amount, 1e-6);
}

/**
 * @brief Transfer should throw when amount exceeds available balance
 *
 * IMPORTANT: Verifies that balances remain unchanged after failure
 */
TEST_F(TransactionServiceTest, Transfer_ThrowsForInsufficientFunds) {
    double fromBefore = accountService.getBalance(FROM_USD_ACCOUNT_ID);
    double toBefore   = accountService.getBalance(TO_USD_ACCOUNT_ID);

    /* Try to send current balance + 100 more money */
    double amount = fromBefore + 100.0;

    /* Expects a insufficient funds error */
    EXPECT_THROW(
        transactionService.transfer(FROM_USD_ACCOUNT_ID, TO_USD_ACCOUNT_ID, amount,
                           "Test transfer - insufficient funds"),
        std::runtime_error
    );

    double fromAfter = accountService.getBalance(FROM_USD_ACCOUNT_ID);
    double toAfter   = accountService.getBalance(TO_USD_ACCOUNT_ID);

    /* Check the balance remain unchanged
     * 0.000001 = 1e-6, tolerance for floating-point comparison
     * how much difference acceptable to still say the values are equal */
    EXPECT_NEAR(fromAfter, fromBefore, 1e-6);
    EXPECT_NEAR(toAfter,   toBefore,   1e-6);
}

/**
 * @brief Transfer should throw error when a negative amount is provided,
 *        and balances must remain unchanged
 */
TEST_F(TransactionServiceTest, Transfer_ThrowsForNegativeAmount) {
    const double amount = -10.0;

    double fromBefore = accountService.getBalance(FROM_USD_ACCOUNT_ID);
    double toBefore   = accountService.getBalance(TO_USD_ACCOUNT_ID);

    EXPECT_THROW(
        transactionService.transfer(FROM_USD_ACCOUNT_ID, TO_USD_ACCOUNT_ID, amount,
                           "Test transfer - negative amount"),
        std::runtime_error
    );

    double fromAfter = accountService.getBalance(FROM_USD_ACCOUNT_ID);
    double toAfter   = accountService.getBalance(TO_USD_ACCOUNT_ID);

    /* Check the balance remain unchanged
     * 0.000001 = 1e-6, tolerance for floating-point comparison
     * how much difference acceptable to still say the values are equal */
    EXPECT_NEAR(fromAfter, fromBefore, 1e-6);
    EXPECT_NEAR(toAfter,   toBefore,   1e-6);
}

/**
 * @brief Transfer should throw error when transferring between different currencies,
 *        and balances must remain unchanged
 */
TEST_F(TransactionServiceTest, Transfer_ThrowsForCurrencyMismatch) {
    const double amount = 5.0;

    /* Get's both account amount before transference */
    double usdBefore = accountService.getBalance(FROM_USD_ACCOUNT_ID);
    double eurBefore = accountService.getBalance(EUR_ACCOUNT_ID);

    EXPECT_THROW(
        transactionService.transfer(FROM_USD_ACCOUNT_ID, EUR_ACCOUNT_ID, amount,
                           "Test transfer - currency mismatch"),
        std::runtime_error
    );

    /* Checks if after the transference */
    double usdAfter = accountService.getBalance(FROM_USD_ACCOUNT_ID);
    double eurAfter = accountService.getBalance(EUR_ACCOUNT_ID);

    /* Check the balance remain unchanged
     * 0.000001 = 1e-6, tolerance for floating-point comparison
     * how much difference acceptable to still say the values are equal */
    EXPECT_NEAR(usdAfter, usdBefore, 1e-6);
    EXPECT_NEAR(eurAfter, eurBefore, 1e-6);
}