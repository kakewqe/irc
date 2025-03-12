#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>

class Client {
public:
    Client(int fd);
    ~Client();

    int getFd() const;
    const std::string& getNickname() const;
    void setNickname(const std::string& nickname);

private:
    int _fd;                
    std::string _nickname;
};

#endif