#include "Client.hpp"

Client::Client(int fd) : _fd(fd) {
}

Client::~Client() {
}

// Getter
int Client::getFd() const {
	return _fd;
}

std::string Client::getNickname() const {
	return _nickname;
}

std::string Client::getUsername() const {
	return _username;
}

bool Client::isInChannel(const std::string& channel) const {
	return _channels.find(channel) != _channels.end();
}

// Setter
void Client::setNickname(const std::string& nickname) {
	_nickname = nickname;
}

void Client::setUsername(const std::string& username) {
	_username = username;
}

// 채널 관련
void Client::joinChannel(const std::string& channel) {
	_channels.insert(channel);
}

void Client::leaveChannel(const std::string& channel) {
	_channels.erase(channel);
}
