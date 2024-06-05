#include "Client.hpp"

Client::Client(int fd) : _fd(fd), _messageBuffer("") {
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

std::string& Client::getMessageBuffer() {
	return _messageBuffer;
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

std::map<std::string, std::string> Client::getBuffer(int fd) {
	std::map<std::string, std::string> buffer;
	char msg[512] = {0};
	int readsize = read(fd, msg, 512);
	if (readsize == 0)
		return buffer;
	else if (readsize < 0)
		return buffer;
	else if (readsize < 512) {
		std::string message(msg);
		message.erase(message.find_last_not_of(" \n\r\t") + 1);
		std::istringstream iss(message);
		std::string command;
		iss >> command;
		if (command == "NICK" || command == "USER" || command == "JOIN" || command == "PART" || command == "PRIVMSG" || command == "KICK" || command == "INVITE" || command == "TOPIC" || command == "MODE" || command == "QUIT") {
			std::string params;
			getline(iss, params);
			params.erase(0, params.find_first_not_of(" "));
			buffer[command] = params;
		}
		else {
			std::string params = "ERROR :Unknown command\r\n";
			buffer[command] = params;
		}
	} else {
		buffer["PRIVMSG"] = "ERROR :Message too long\r\n";
	}
	return buffer;
}