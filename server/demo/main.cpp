#include "server.hpp"

int main(int argc, char* argv[]){
    /* default listening port if none is parsed on arg */
    int port = 8080;

    /* refreshes the port if any is parsed on arg */
    if (argc >= 2) {
        try {
            port = std::stoi(argv[1]);
        } catch (const std::exception& e) {
            /* if invalid argument is parsed print an error */
            std::cerr << "Invalid port number: " << argv[1] << "\n";
            return 1;
        }
    }

    /* Initializes the server instance parsing the chosen port to it */
    Server server(port);

    /* starts the server, returning its return code */
    return server.run();
}
