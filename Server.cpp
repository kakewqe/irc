#include "Server.hpp"
#include "Client.hpp"

Server::Server(int port, const std::string& password) : _port(port), _password(password), _serverFd(-1) 
{
    initializ_socket();
}

Server::~Server() 
{
    stop();
}

void Server::initializ_socket() 
{
    _serverFd = socket(AF_INET, SOCK_STREAM, 0);
    if (_serverFd < 0)
        throw std::runtime_error("Failed to create socket");
    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(_port);

    if (bind(_serverFd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) 
    {
        close(_serverFd);
        throw std::runtime_error("Failed to bind socket");
    }

    if (listen(_serverFd, SOMAXCONN) < 0) 
    {
        close(_serverFd);
        throw std::runtime_error("Failed to listen on socket");
    }

    pollfd serverPollFd;
    serverPollFd.fd = _serverFd;
    serverPollFd.events = POLLIN;
    _pollFds.push_back(serverPollFd);

    std::cout << "Server started on port " << _port << std::endl;
}

void Server::start() 
{
    while (true) 
    {
        int pollCount = poll(_pollFds.data(), _pollFds.size(), -1);
        if (pollCount < 0) 
            throw std::runtime_error("Poll error");

        for (size_t i = 0; i < _pollFds.size(); ++i) 
        {
            if (_pollFds[i].revents & POLLIN) 
            {
                if (_pollFds[i].fd == _serverFd) 
                    Management_connection();
                else 
                    Management_client(_pollFds[i].fd);
            }
        }
    }
}

void Server::Management_connection() 
{
    struct sockaddr_in clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);
    int clientFd = accept(_serverFd, (struct sockaddr*)&clientAddr, &clientAddrLen);
    if (clientFd < 0) 
    {
        std::cerr << "Failed to accept client connection" << std::endl;
        return;
    }

    Client newClient(clientFd);

    std::string requestNick = "Please enter a nickname: ";
    send(clientFd, requestNick.c_str(), requestNick.size(), 0);

    char buffer[1024];
    ssize_t bytesRead = recv(clientFd, buffer, sizeof(buffer) - 1, 0);
    if (bytesRead <= 0) 
    {
        std::cerr << "Failed to receive nickname" << std::endl;
        close(clientFd);
        return;
    }

    buffer[bytesRead] = '\0';
    std::string nick(buffer);

    if (nick.empty())
        nick = "client_" + std::to_string(clientFd);

    newClient.setNickname(nick); 

    _clients.push_back(newClient);

    pollfd clientPollFd;
    clientPollFd.fd = clientFd;
    clientPollFd.events = POLLIN;
    _pollFds.push_back(clientPollFd);

    std::cout << "New client connected: " << inet_ntoa(clientAddr.sin_addr) 
              << " with nickname: " << newClient.getNickname() << std::endl;
}

void Server::Management_client(int clientFd) 
{
    char buffer[1024];
    ssize_t bytesRead = recv(clientFd, buffer, sizeof(buffer) - 1, 0);
    
    if (bytesRead <= 0) 
    {
        removeClient(clientFd);
        return;
    }

    buffer[bytesRead] = '\0';

    std::cout << "Message reçu du client " << clientFd << ": " << buffer << std::endl;

    if (strstr(buffer, "NICK") == buffer) 
    {
        
    }

    std::string response = "Message reçu: " + std::string(buffer) + "\r\n";
    send(clientFd, response.c_str(), response.size(), 0);
}


void Server::removeClient(int clientFd) 
{
    close(clientFd);
    
    for (std::vector<Client>::iterator it = _clients.begin(); it != _clients.end(); ++it) 
    {
        if (it->getFd() == clientFd) {
            _clients.erase(it);
            break;
        }
    }

    for (std::vector<struct pollfd>::iterator it = _pollFds.begin(); it != _pollFds.end(); ++it) 
    {
        if (it->fd == clientFd) {
            _pollFds.erase(it);
            break;
        }
    }

    std::cout << "Client disconnected" << std::endl;
}


void Server::stop() 
{
    for (std::vector<Client>::iterator it = _clients.begin(); it != _clients.end(); ++it) 
    {
        close(it->getFd());
    }

    close(_serverFd);
    std::cout << "Server stopped" << std::endl;
}