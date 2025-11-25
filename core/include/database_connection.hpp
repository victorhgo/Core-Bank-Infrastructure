#ifndef DB_CONNECTION_HPP
#define DB_CONNECTION_HPP

/* Includes */
#include <iostream>
/* libpqxx */
#include <pqxx/pqxx>
/* Handles string */
#include <string>
#include <sstream>
/* handles file stream */
#include <fstream>
/* Error handling library */
#include <stdexcept>
/* to use std::mutex */
#include <mutex>

/**
 * @class db_connection
 *
 * @brief This class is responsible for abstracting some functionalities like
 * loading database configuration, managing a PostgreSQL connection (thru libpqxx), 
 * and also performing some basic query transactions
 *
 * This class can be thought as the foundation of the Data Access Layer:
 * It loads database parameters from a JSON configuration file, builds
 * a connection string then initializes a single libpqxx connection shared
 * across the entire application
 *
 */
class DBConnection {
public:
    /**
     * @brief Retrieve the unique (global) singleton instance of 
     * the database connection
     *
     * Ensures that only a single database connection exists in the program.
     * All DAL services must call this function to access the shared connection
     *
     * @return A reference to the singleton db_connection instance
     */
    static DBConnection& getInstance();

    /**
     * @brief Load database credential and config parms from a JSON file
     *
     * @param path Path to the configuration file.
     * It can also return a std::runtime_error if the file cannot be read 
     * or contains invalid fields. */
    void loadConfig(const std::string& path);

    /**
     * @brief Function to stabilish a connection to the database
     *  
     * Returns an error std::runtime_error iff the connection fails */
    void connect();

    /**
     * @brief Checks if the database connection is currently open
     * 
     * @return true if the connection is open
     * @return false otherwise
     */
    bool isConnected() const;

    /**
     * @brief Get the Connection object (pqxx::connection object)
     * 
     * @return reference to object, pqxx::connection& 
     */
    pqxx::connection& getConnection();

    /**
     * @brief Create a Write Transaction object (pqxx::work)
     * 
     * This function will be used for INSERT, UPDATE, DELETE operations
     *
     * @return std::unique_ptr<pqxx::work> 
     */
    std::unique_ptr<pqxx::work> createWriteTransaction();

    /**
     * @brief Create a Read Transaction object (pqxx::read_transaction)
     * 
     * This function will be used for SELECT queries. 
     * Note that: Attempting to modify data will cause errors.
     * @return std::unique_ptr<pqxx::read_transaction> 
     */
    std::unique_ptr<pqxx::read_transaction> createReadTransaction();

    /**
     * @brief Acquire a scoped lock for DB operations in multithreaded contexts
     *
     * Any code path that uses the shared pqxx::connection from multiple threads
     * should hold this lock to avoid concurrent access
     */
    std::unique_lock<std::mutex> lock();

private:
    /**
     * @brief Private constructor for Singleton pattern.
     *
     * This prevents external instantiation. Configuration must be loaded
     * first using loadConfig() and only then connect() can be called
     */
    DBConnection() = default;

    /**
     * @brief deletes copy constructor to prevent copying of singleton
     * 
     */
    DBConnection(const DBConnection&) = delete;

    /**
     * @brief deletes assignment operator to prevent copy of singleton
     * 
     * @return DBConnection& 
     */
    DBConnection& operator=(const DBConnection&) = delete;

    /**
     * @brief Constructed PostgreSQL connection string in libpq format
     * 
     */
    std::string connectionString;
    
    /**
     * @brief pointer to the active PostgreSQL connection, 
     * set when connect() is called
     * 
     */
    std::unique_ptr<pqxx::connection> conn;

    /* --- These are the configuration fields are loaded by loadConfig() --- */
    
    /** @brief Database host (def is "localhost") */
    std::string host;

    /** @brief TCP port of the DB server (def is 5432) */
    int port = 5432;

    /** @brief Name of the PostgreSQL database (we'll be using database1)*/
    std::string dbname;

    /** @brief Username for authentication (def is postgres) */
    std::string user;

    /** @brief Password for authentication (def is no pass) */
    std::string password;

    /** @brief SSL mode (def is "disable") */
    std::string sslmode;

    /** @brief Timeout (seconds) for connection attempt */
    int connect_timeout = 5;

    mutable std::mutex dbMutex;
};

#endif