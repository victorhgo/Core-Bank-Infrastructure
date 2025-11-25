/* Design for a minimal Transaction Server */
#ifndef SERVER_HPP
#define SERVER_HPP

#include <atomic>
#include <thread>
#include <vector>
#include <string>

/**
 * @class TransactionServer
 *
 * @brief Scope for the minimal long running TCP server that wraps the 
 * bank core as a simple text-based transaction service
 *
 * This server listens on a TCP port and accepts connections from
 * external clients, where each sends commands like:
 *
 *   - PING
 *   - BALANCE <accountID>
 *   - TRANSFER <fromID> <toID> <amount> <description>
 *
 * For each connected client, the server spawns a worker thread that reads
 * commands, delegates to the Data Access Layer and returns responses. 
 * Using DB Connection and relying on concurrency primitives (threads,
 * atomic flags and mutexes inside DBConnection) to remain safe when multiple
 * clients are active at the same time
 */
class Server {
    public:
        /**
         * @brief Constructs a new Server instance
         *
         * This constructor only stores the host and port configuration and **does
         * not open any sockets yet**. To actually start listening for connections,
         * call start() method
         *
         * @param host Hostname or IP address to bind to
         * @param port TCP port number on which the server will listen for clients (def is 8080)
         */
        Server(const std::string& host, int port);

        /**
         * @brief Destructor for Server
         *
         * If the server is still running when the destructor is called, calls  stop() to shut down 
         * the listening socket and join the accept thread ensuring a clean shutdown
         */
        ~Server();

        /**
         * @brief Start listening for client connections by creating the listening socket
         * If the server is already running, this function returns immediately
         *
         * Throwns an error if the socket cannot be created or put into listening mode
         */
        void start();

        /**
         * @brief Stops the server and clean resources
         * 
         */
        void stop();
    private:

        /**
         * @brief Main loop that waits for incoming client connections
         * 
         * Loops continue until running flag is cleared and the stop() method is called
         */
        void acceptLoop();

        /**
         * @brief Handles a single connection
         * 
         * @param clientSocket File descriptor for the accepted client socket
         */
        void handleClient(int clientSocket);

        /**
         * @brief  Host and IP address the server will bind to
         * default: 127.0.0.1
         */
        std::string hostBind;

        /**
         * @brief TCP port number the server will bind to
         * default: 8080
         */
        int portBind;

        /**
         * @brief File descriptor for the listening socket
         * 
         */
        int listenSocket = -1;

        /** Flag to indicate if the server is running or not */
        std::atomic<bool> running{false};

        /**
         * @brief Accept loop thread
         *
         * This thread runs acceptLoop() while the server is running
         */
        std::thread acceptThread;

        /**
         * @brief Optional container to track worker threads
         * 
         */
        std::vector<std::thread> workerThread;
};


#endif