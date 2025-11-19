#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>

/* deals with strings*/
#include <string>

/* to use the std::memset function */
#include <cstring>

/* socket(), bind(), listen(), accept(), setsockopt */
#include <sys/socket.h>

/* inet_addr, htons, htonl and other inet functionalities */
#include <arpa/inet.h>

/* sockaddr_in */
#include <netinet/in.h>

/*deals with basic system data types */
#include <sys/types.h>

/* close()*/
#include <unistd.h>

/**
 * @brief This class encapsulates a simple single threaded HTTP server just using
 * UNIX sockets
 * 
 * It listens on the specified port and accepts one connection at a time, reading
 * the request and returning a fixed HTML response */
class Server {
public:

    /**
     * @brief Construct a new Server object that listen on the given TCP port
     * 
     * @param port given TCP port
     */
    Server(int port);
    
    /**
     * @brief Destroy the Server object if still open
     * 
     */
    ~Server();

    /**
     * @brief Starts the server
     * Initialize the listening socket init() and enter on an infinite
     * accept loop, for each accepted client, handleClient() is called
     * 
     * @return int, 0 if run successfully, otherwise if init fails
     */
    int run();

    /**
     * @brief Disables a copy construction and a copy assignment
     * Since the server owns the file descriptor, it would be unsafe
     * to copying it
     */
    Server(const Server&) = delete;
    Server& operator=(const Server&) = delete;

private:
    /* portm stores which TCP port the server will listen on
     * serverfd the socket file descriptor for the listening socket */
    int portm;
    int serverfd;
    
    /**
     * @brief Initialize the listening socket, binds it and listen
     * 
     * @return true on success
     * @return false on error and prints it to stderr
     */
    bool init();

    /**
     * @brief Handles a single client connection
     * reads the request, print it for debug, sends back HTML page,
     * then closes the client socket
     * @param clientfd client file descriptor
     */
    void handleClient(int clientfd);
};

#endif