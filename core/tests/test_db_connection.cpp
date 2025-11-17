/* Here we will implement some unit tests for DB Connection using
Google Test framework. Include tests like

    - test the failure cases (wrong credential files, non existing credentials)
    - Test the connection
    - Test a simple query like we did on demo */
#include <gtest/gtest.h>
#include "database_connection.hpp"

/**
 * @class DBConnectionTest
 * 
 * @brief GoogleTest tool for integration tests of the DBConnection class
 *
 * This tool will ensure before each test the database connection is 
 * properly initialized and ready to use using the DBConnection::getInstance()
 * method
 * 
 */
class DBConnectionTest : 

public ::testing::Test {

    protected:

        void SetUp() override {
            // Make sure config is loaded and DB is connected
            auto& db = DBConnection::getInstance();

            if (!db.isConnected()) {
                db.loadConfig("config/db_credential.json");
                db.connect();
            }
        }
};

/**
 * @brief Tests that connecting with invalid credentials fails.
 *
 * Uses a separate JSON file with wrong username/password.
 */
TEST(DBConnectionFailureTest, ConnectThrowsOnInvalidCredentials) {
    auto& db = DBConnection::getInstance();

    /* We need to ensure we're not already connected. */
    if (db.isConnected()) {
        GTEST_SKIP() << "DB already connected, we cannot test invalid credentials.";
    }

    /* Load the bad credential json */
    ASSERT_NO_THROW(
        db.loadConfig("config/db_badcredential.json")
    ) << "Failed to load bad credentials config file";

    /* Trying to connect using the wrong credentials will fail */
    EXPECT_THROW(
        db.connect(),
        std::runtime_error
    );

    /* If the connection didn't work, we need to be disconnected */
    EXPECT_FALSE(db.isConnected());
}

/**
 * @brief Tests that loading a non existing config file does not work
 */
TEST(DBConnectionFailureTest, LoadConfigThrowsOnInvalidPath) {
    auto& db = DBConnection::getInstance();

    /* Using a file that definitely does not exist */
    EXPECT_THROW(
        db.loadConfig("config/non_existing.json"),
        std::runtime_error
    );
}

/**
 * @brief Construct a new test f object for testing the Connection
 * with database
 * 
 */
TEST_F(DBConnectionTest, ConnectsSuccessfully) {
    auto& db = DBConnection::getInstance();
    EXPECT_TRUE(db.isConnected());
}

/**
 * @brief Construct a new test f object for testing a
 * a read transaction for at least one row
 * 
 */
TEST_F(DBConnectionTest, CanRunSimpleSelect) {
    auto& db = DBConnection::getInstance();
    auto tx = db.createReadTransaction();

    /* expect at least one customer row */
    pqxx::result res = tx->exec("SELECT customer_id, full_name FROM customers LIMIT 1;");
    tx->commit();

    EXPECT_GT(res.size(), 0u);
}

/**
 * @brief Construct a new test f object for testing a
 * read transaction and expects exactly one customer
 */
TEST_F(DBConnectionTest, CanFetchSpecificCustomerById) {
    auto& db = DBConnection::getInstance();
    auto tx = db.createReadTransaction();

    /* check customer_id = 2 exists and has non-empty name */
    pqxx::result res = tx->exec(
        "SELECT full_name, email FROM customers WHERE customer_id = $1",
        2
    );

    /* Commits the transaction */
    tx->commit();

    ASSERT_EQ(res.size(), 1u) << "Expected exactly one customer with id = 2";

    std::string name  = res[0]["full_name"].as<std::string>();
    std::string email = res[0]["email"].as<std::string>();

    EXPECT_FALSE(name.empty());
    EXPECT_FALSE(email.empty());
}