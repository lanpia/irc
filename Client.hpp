#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <set>

class Client {
public:
    Client(int fd);
    ~Client();

    int getFd() const;
    std::string getNickname() const;
    void setNickname(const std::string& nickname);
    std::string getUsername() const;
    void setUsername(const std::string& username);
    void joinChannel(const std::string& channel);
    void leaveChannel(const std::string& channel);
    bool isInChannel(const std::string& channel) const;

private:
    int _fd;
    std::string _nickname;
    std::string _username;
    std::set<std::string> _channels;
};

#endif // CLIENT_HPP
