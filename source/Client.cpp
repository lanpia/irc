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
	char msg[513] = {0};
	int readsize = read(fd, msg, 512);
	if (readsize <= 0)
		return buffer;
	msg[readsize] = '\0';
	std::string message = getMessageBuffer();
	message = message.append(msg, readsize);
	if (message.empty()) {
		return buffer;
	}
	if (message.size() == 512) {
		buffer["PRIVMSG"] = " :Message too long\r\n";
		return buffer;
	}
	// size_t pos = message.find("\r\n");
	// if (pos == std::string::npos) {
	// 	buffer["ERROR"] =  " :message is not end of newline\r\n";
	// 	return buffer;
	// }
	message = message.substr(0, message.find("\r\n"));
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
		buffer["ERROR"] =  " :Unknown command\r\n";
	}
	return buffer;
}