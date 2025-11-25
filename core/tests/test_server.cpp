/* These integration tests for the transaction Server using GoogleTest assumes:
 *
 *  - DB is running
 *  - json credential is valid
 *  - accounts and customers tables exist
 *  - at least one account row exists */
#include <gtest/gtest.h>

#include "database_connection.hpp"
#include "server.hpp"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <chrono>
#include <cstring>
#include <sstream>
#include <string>
#include <thread>
#include <memory>
#include <vector>
#include <cmath>

namespace {
    constexpr int TEST_PORT = 5555;
}

/**
 * @brief GoogleTest tool for integration tests of the Server class.
 *
 * Starts a Server on localhost:TEST_PORT before each test then stops it
 * Provides a helper to send a single command and read the response line from server
 */
class ServerTest : public ::testing::Test {
    protected:
        void SetUp() override {
            /* Using DBConnection to ensure connection to the DB */
            auto& db = DBConnection::getInstance();
            if (!db.isConnected()) {
                db.loadConfig("config/db_credential.json");
                db.connect();
            }

            /* Start the server on a test port 5555 deft */
            server = std::make_unique<Server>("127.0.0.1", TEST_PORT);
            server->start();

            /* Small sleep to give the acceptLoop time to enter in listen/accept mode */
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        /* tear down the test */
        void TearDown() override {
            if (server) {
                server->stop();
                server.reset();
            }
        }

        /**
        * @brief Helper to send a single command to the server and get one response line
        *
        * Opens a new TCP connection, sends a command that read up to 1024 bytes then closes 
        * the socket returning the response string
        */
        std::string sendCommand(const std::string& cmd) {
            int sock = ::socket(AF_INET, SOCK_STREAM, 0);
            if (sock < 0) {
                throw std::runtime_error("socket() failed");
            }

            sockaddr_in addr{};
            addr.sin_family = AF_INET;
            addr.sin_port   = htons(TEST_PORT);
            addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK); // 127.0.0.1

            if (::connect(sock, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
                ::close(sock);
                throw std::runtime_error("connect() failed");
            }

            std::string line = cmd + "\n";
            ssize_t sent = ::send(sock, line.c_str(), line.size(), 0);
            if (sent < 0) {
                ::close(sock);
                throw std::runtime_error("send() failed");
            }

            char buffer[1024];
            std::memset(buffer, 0, sizeof(buffer));
            ssize_t n = ::recv(sock, buffer, sizeof(buffer) - 1, 0);
            if (n < 0) {
                ::close(sock);
                throw std::runtime_error("recv() failed");
            }

            std::string resp(buffer, n);

            while (!resp.empty() && (resp.back() == '\n' || resp.back() == '\r')) {
                resp.pop_back();
            }

            ::close(sock);
            return resp;
        }

        std::unique_ptr<Server> server;p
};

/**
 * @test PING should return PONG
 */
TEST_F(ServerTest, PingReturnsPong) {
    std::string resp = sendCommand("PING");
    EXPECT_EQ(resp, "PONG");
}

/**
 * @test BALANCE for an existing account should return a BALANCE line
 *
 * We only assert on the format and account ID but not the exact amount because
 * other tests can changes the balance
 */
TEST_F(ServerTest, Balance_ReturnsBalanceForExistingAccount) {
    /* Assume account 1 exists */
    std::string resp = sendCommand("BALANCE 1");

    /* response should look like "BALANCE 1 <amount> */
    std::istringstream iss(resp);
    std::string tag;
    int accId;
    double amount;

    iss >> tag >> accId >> amount;

    EXPECT_EQ(tag, "BALANCE");
    EXPECT_EQ(accId, 1);

    /* checks if a numeric amount was parsed */
    EXPECT_FALSE(iss.fail());
}

/**
 * @test BALANCE for a non existing account should yield an ERROR
 */
TEST_F(ServerTest, Balance_ReturnsErrorForNonExistingAccount) {
    std::string resp = sendCommand("BALANCE 999999");
    /* We just expect the response to start with ERROR */
    ASSERT_GE(resp.size(), 5u);
    EXPECT_EQ(resp.substr(0, 5), "ERROR");
}

/**
 * @test TRANSFER should always be successful for allowed amount then adjust balances
 *
 * Uses only the server protocol by asking for BALANCE, performing the TRANSFER 
 * then checks new BALANCEs by asking BALANCE...
 */
TEST_F(ServerTest, Transfer_SucceedsAndChangesBalances) {
    /* Get initial balances for accounts 1 and 2 */
    auto parseBalance = [this](int accountId) -> double {
        std::string resp = sendCommand("BALANCE " + std::to_string(accountId));
        std::istringstream iss(resp);
        std::string tag;
        int id;
        double amt;
        iss >> tag >> id >> amt;
        if (iss.fail() || tag != "BALANCE" || id != accountId) {
            throw std::runtime_error("Unexpected BALANCE response: " + resp);
        }
        return amt;
    };

    double fromBefore = parseBalance(1);
    double toBefore   = parseBalance(2);

    /* Choose a small amount fromBefore */
    double amount = fromBefore * 0.1;
    if (amount < 1.0) {
        amount = 1.0;
    }

    /* If source accnt has no money we skip the test */
    if (fromBefore <= 0.0) {
        GTEST_SKIP() << "Source account 1 has non-positive balance, cannot test transfer.";
    }

    std::ostringstream cmd;
    cmd << "TRANSFER 1 2 " << amount;

    std::string resp = sendCommand(cmd.str());
    ASSERT_FALSE(resp.empty());
    if (resp.rfind("ERROR", 0) == 0) {
        FAIL() << "TRANSFER returned error: " << resp;
    }
    EXPECT_EQ(resp, "OK");

    double fromAfter = parseBalance(1);
    double toAfter   = parseBalance(2);

    EXPECT_NEAR(fromAfter, fromBefore - amount, 1e-2);
    EXPECT_NEAR(toAfter,   toBefore   + amount, 1e-2);
}

/**
 * @test TRANSFER should return ERROR when amount exceeds balance,
 *       and balances must remain unchanged
 */
TEST_F(ServerTest, Transfer_InsufficientFunds_ReturnsErrorAndKeepsBalances) {
    auto parseBalance = [this](int accountId) -> double {
        std::string resp = sendCommand("BALANCE " + std::to_string(accountId));
        std::istringstream iss(resp);
        std::string tag;
        int id;
        double amt;
        iss >> tag >> id >> amt;
        if (iss.fail() || tag != "BALANCE" || id != accountId) {
            throw std::runtime_error("Unexpected BALANCE response: " + resp);
        }
        return amt;
    };

    double fromBefore = parseBalance(1);
    double toBefore   = parseBalance(2);

    /* sends more than balance */
    double amount = fromBefore + 1000.0;

    std::ostringstream cmd;
    cmd << "TRANSFER 1 2 " << amount;

    std::string resp = sendCommand(cmd.str());
    ASSERT_GE(resp.size(), 5u);
    EXPECT_EQ(resp.substr(0, 5), "ERROR");

    double fromAfter = parseBalance(1);
    double toAfter   = parseBalance(2);

    EXPECT_NEAR(fromAfter, fromBefore, 1e-6);
    EXPECT_NEAR(toAfter,   toBefore,   1e-6);
}

/**
 * @test Concurrent BALANCE requests should all return valid BALANCE lines
 * even when multiple clients hit the server at the same time
 */
TEST_F(ServerTest, ConcurrentBalanceRequests_ReturnConsistentFormat) {
    const int numThreads = 10;

    std::vector<std::thread> threads;
    std::vector<std::string> responses(numThreads);

    /* Spawns several threads, each calling sendCommand("BALANCE 1") */
    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back([this, i, &responses]() {
            responses[i] = sendCommand("BALANCE 1");
        });
    }

    /* Join all threads */
    for (auto& t : threads) {
        t.join();
    }

    /* Check all responses as they should all look like "BALANCE 1 <amount>" */
    for (const auto& resp : responses) {
        std::istringstream iss(resp);
        std::string tag;
        int id;
        double amt;

        iss >> tag >> id >> amt;

        EXPECT_EQ(tag, "BALANCE");
        EXPECT_EQ(id, 1);
        EXPECT_FALSE(iss.fail());
    }
}

/**
 * @test Several concurrent TRANSFER operations from account 1 to 2
 * must result in a total sum of all transfers
 *
 * Must use only the server protocol with multiple threads each sending
 * a single TRANSFER command, to validate it works
 */
TEST_F(ServerTest, ConcurrentTransfers_AreConsistentWithTotalAmount) {
    auto parseBalance = [this](int accountId) -> double {
        std::string resp = sendCommand("BALANCE " + std::to_string(accountId));
        std::istringstream iss(resp);
        std::string tag;
        int id;
        double amt;
        iss >> tag >> id >> amt;
        if (iss.fail() || tag != "BALANCE" || id != accountId) {
            throw std::runtime_error("Unexpected BALANCE response: " + resp);
        }
        return amt;
    };

    double fromBefore = parseBalance(1);
    double toBefore   = parseBalance(2);

    const int numTransfers = 1000;

    /* Choose a valid amount which is a small fraction of fromBefore and rounded to cents
     * because DB uses NUMERIC(12,2)... */
    double rawAmount = fromBefore / (numTransfers * 4.0); // quarter of what the user can spend
    double amount = std::floor(rawAmount * 100.0) / 100.0;

    if (amount <= 0.0) {
        GTEST_SKIP() << "Source account 1 has too small balance for concurrency test.";
    }

    std::vector<std::thread> threads;

    /* Fire numTransfers concurrent TRANSFER commands */
    for (int i = 0; i < numTransfers; ++i) {
        threads.emplace_back([this, amount]() {
            std::ostringstream cmd;
            cmd << "TRANSFER 1 2 " << amount;
            std::string resp = sendCommand(cmd.str());
            /* note that we are not asserting inside worker threads because the main thread checks each aggregate */

        });
    }

    for (auto& t : threads) {
        t.join();
    }

    double fromAfter = parseBalance(1);
    double toAfter   = parseBalance(2);

    double expectedFrom = fromBefore - numTransfers * amount;
    double expectedTo   = toBefore   + numTransfers * amount;

    /* Because of cent rounding in NUMERIC(12,2),
     * we accept a small epsilon at the cent level. */
    EXPECT_NEAR(fromAfter, expectedFrom, 1e-2);
    EXPECT_NEAR(toAfter,   expectedTo,   1e-2);
}