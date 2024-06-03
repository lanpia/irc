#include "Client.hpp"

Client::Client(int fd) : _fd(fd) {
}

Client::~Client() {
}

int Client::getFd() const {
    return _fd;
}

std::string Client::getNickname() const {
    return _nickname;
}

void Client::setNickname(const std::string& nickname) {
    _nickname = nickname;
}

std::string Client::getUsername() const {
    return _username;
}

void Client::setUsername(const std::string& username) {
    _username = username;
}

void Client::joinChannel(const std::string& channel) {
    _channels.insert(channel);
}

void Client::leaveChannel(const std::string& channel) {
    _channels.erase(channel);
}

bool Client::isInChannel(const std::string& channel) const {
    return _channels.find(channel) != _channels.end();
}
