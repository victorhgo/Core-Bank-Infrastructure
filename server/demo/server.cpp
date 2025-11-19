#include "server.hpp"

/* The constructor store the port and mark serverfd as invalid (-1) */
Server::Server(int port) 
    : portm(port), serverfd(-1) {
}

/* The destructor closes the connection before destroying it */
Server::~Server() {
    if (serverfd >= 0) {
	/* Ensures a connection is always closed 
	 * when destroying the server connection instance */    
        close(serverfd);
    }
}

bool Server::init() {
    /* Creating a TCP socket (IPv4, Stream oriented)*/
    serverfd = socket(AF_INET, SOCK_STREAM, 0);

    /* Prints error if not successful on creating the TCP socket */
    if (serverfd < 0) {
        perror("socket");

	return false;
    }
    
    /* To allow a quick restart in the same port
     * Avoids the Address already in use in case of restar */
    int opt = 1;

    if (setsockopt(serverfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        close(serverfd);
        serverfd = -1;

        return false;
    }

    /* lets prepare the sockaddr_in struct with a IPv4 (AF_INET)
     * a INADDR_ANY (0.0.0.0 to listen on all network interfaces)
     * chosen port in network byte order htons
     */
    sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    /* host byte order */
    addr.sin_port = htons(portm);

    /* Binds the socket to the address and port */
    if(bind(serverfd, (sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(serverfd);
        serverfd = -1;

	    return false;
    }

    /* To mar the socket as a listening one, where 10 is the max pending
     * connection queue lenght */
    if (listen(serverfd, 10) < 0) {
        perror("listen");
        close(serverfd);
        serverfd = -1;

        return false;
    }

    /* Socket is initialized and ready */
    return true;
}

void Server::handleClient(int clientfd) {
    /* Reads an HTTP request in the most naive form, 
     * buffer to hold the incoming HTTP request */
    char buffer[4096];

    /* We need to clear the buffer to ensure it is null terminated after the recv */
    std::memset(buffer, 0, sizeof(buffer));

    /**
     * Receives data from the client, where
     * clientfd is the client connected socket
     * buffer is where to store the data
     * sizeof(buffer) to leave space for closing it
     * 0 - no special behavior
     */
    ssize_t bytes_read = recv(clientfd, buffer, sizeof(buffer) - 1, 0);

    if (bytes_read < 0) {
        perror("recv");
        close(clientfd);

        return;
    }

    /* Convers the "raw C string" to std::string for easier handling */
    std::string request(buffer);

    /* Prints the HTTP request to the console */
    std::cout << "Received request: " << request << "\n";

    /* Build a simple HTTP response */
    std::string body = R"(<!DOCTYPE html>
<html>
<head><title>Demo C++ Tiny Server</title></head>
<body>
    <h1> Hello World! From my C++ Server! :D</h1>
    <p> This is a tiny HTTP server using UNIX raw sockets</p>
    <p> Thank you very much, Victor Correa</p>
</body>
</html>
)";

    /* Builds the full HTTP response such as*/
    std::string response;

    /* status line displays http version and status code, reason phrase*/
    response += "HTTP/1.1 200 OK\r\n";
    response += "Content-Type: text/html; charset=UTF-8\r\n";
    response += "Content-Length: " + std::to_string(body.size()) + "\r\n";
    response += "Connection: close\r\n";
    response += "\r\n";

    /* Appends the HTML body */
    response += body;

    /* Sends a response and closes the connection right after 
     * Here the loop is useful in case send() does not send all bytes in
     * one call, so we loop until the entire response is sent (or error occurs)*/
    ssize_t total_sent = 0;
    while (total_sent < (ssize_t)response.size()) {
        ssize_t sent = send(clientfd,
                            response.data() + total_sent,
                            response.size() - total_sent,
                            0);
        if (sent < 0) {
            perror("send");
            break;
        }

        total_sent += sent;
    }

    /* Closes the connection to the client */
    close(clientfd);

}

int Server::run() {
    /* Initializes the listening socket */
    if (!init()) {
	    return 1;
    }
    
    /* Display which port is listening on */
    std::cout << "HTTP server listening on port " << portm << "...\n";

    /* Accepts the loop, but only ONE request per connection in sequential way */
    while (true) {
        /* struct to hold client's address info */
        sockaddr_in client_addr;

        /* socklen_t initially contains the size of client address but accept()
         * will update it with the actual used size */
	    socklen_t client_len = sizeof(client_addr);

        /* accept block until a client connections, if success, returns a
         * new file descriptor clientfd connected to hte client */
	    int clientfd = accept(serverfd, (sockaddr*)&client_addr, &client_len);

        if (clientfd < 0) {
            perror("accept");

            /* Tries the next connection */
            continue;
        }

        /* Handles the accepted client connection sequentially */
        handleClient(clientfd);
    }
    
    return 0;
}

