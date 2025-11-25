/* Develop our functions and class implementation */
#include "database_connection.hpp"
#include "json.hpp"

using json = nlohmann::json;

DBConnection& DBConnection::getInstance() {
    /* static var in C++: ensures this var is initialized exactly once:
     * the first time the function is called. All future calls returns the
     * same instance, ensuring that the application uses only one DB connection*/
    static DBConnection instance;
    return instance;
}

void DBConnection::loadConfig(const std::string& path) {
    std::ifstream file(path);

    /* Check if config file is accessible */
    if (!file.is_open())
        throw std::runtime_error("Failed to open config file: " + path);

    /* parses the .json content */
    json cfg;
    file >> cfg;
    
    /* Standard initialization */
    host = cfg.value("host", "");
    port = cfg.value("port", 0);
    dbname = cfg.value("dbname", "");
    user = cfg.value("user", "");
    password = cfg.value("password", "");
    sslmode = cfg.value("sslmode", "");
    connect_timeout = cfg.value("connect_timeout", 0);

    /* Builds PostgreSQL connection string loading from json */
    std::ostringstream ss;
    ss << "host=" << host
       << " port=" << port
       << " dbname=" << dbname
       << " user=" << user
       << " password=" << password
       << " sslmode=" << sslmode
       << " connect_timeout=" << connect_timeout;

    connectionString = ss.str();
}

void DBConnection::connect() {
    /* if connection exists, do nothing */
    if (conn && conn->is_open())
        return;

    try {
        /* creates a new postgreSQL connection */
        conn = std::make_unique<pqxx::connection>(connectionString);

        /* checks the whether connection is successfully or not*/
        if (!conn->is_open()) {
            throw std::runtime_error("Database connection failed.");
        }
    }
    catch (const std::exception& e) {
        /* throws the exception runtime_error */
        throw std::runtime_error(std::string("Connection error: ") + e.what());
    }
}

bool DBConnection::isConnected() const {
    return conn && conn->is_open();
}

pqxx::connection& DBConnection::getConnection() {
    /* If not connect, throw a runtime_error */
    if (!isConnected())
        throw std::runtime_error("Database not connected!");
    return *conn;
}

std::unique_ptr<pqxx::work> DBConnection::createWriteTransaction() {
    /* a unique pointer to a newly created pqxx::work transaction */
    return std::make_unique<pqxx::work>(getConnection());
}

std::unique_ptr<pqxx::read_transaction> DBConnection::createReadTransaction() {
    /* a unique pointer to a newly created pqxx::read_transaction query*/
    return std::make_unique<pqxx::read_transaction>(getConnection());
}

std::unique_lock<std::mutex> DBConnection::lock() {
    return std::unique_lock<std::mutex>(dbMutex);
}