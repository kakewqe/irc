#include <string>
#include <iostream>
#include <poll.h>
#include <vector>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring> //a retirer

class   Client;

class Server
{

    public:

        Server(int port, const std::string &password);
        ~Server();
        void start();
        void stop();
    
    private:
    
        void    initializ_socket();
        void    Management_connection();
        void    Management_client(int clientFd);
        void    removeClient(int clientFd);

        int _port;
        std::string _password;
        int _serverFd;
        std::vector<Client> _clients;
        std::vector<pollfd> _pollFds;
};