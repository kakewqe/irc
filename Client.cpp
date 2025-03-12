#include "Client.hpp"

Client::Client(int fd) : _fd(fd), _nickname("") {}

Client::~Client() {}

int Client::getFd() const 
{
    return _fd;
}

const std::string& Client::getNickname() const 
{
    return _nickname;
}

void Client::setNickname(const std::string& nickname) 
{
    _nickname = nickname;
}