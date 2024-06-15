/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nahyulee <nahyulee@student.42seoul.kr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/08 19:50:01 by nahyulee          #+#    #+#             */
/*   Updated: 2024/06/15 18:12:35 by nahyulee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "Client.hpp"

Client::Client() {}
Client::Client(const Client& copy) { *this = copy; }
Client& Client::operator=(const Client& copy) { (void) copy; return *this; }
Client::Client(int fd) : fd(fd) {
	this->ClientInfo[Nickname] = "";
	this->ClientInfo[Username] = "";
	this->ClientInfo[InChannel] = "false";
	this->ClientInfo[Operator] = "false";
}
Client::~Client() {}

bool Client::isValidNickname(const std::string& nickname) const {
	if (nickname.size() == 0 || nickname.size() > 9)
		return false;
	for (size_t i = 0; i < nickname.size(); i++) {
		if (!isalnum(nickname[i]))
			return false;
	}
	if (nickname == "root" || nickname == "admin")
        return false;
	return true;
}

int Client::getFd() const {
	return this->fd;
}

std::string Client::is(enum e_info idx) const {
	return this->ClientInfo[idx];
}

void Client::set(enum e_info idx, const std::string opt, const std::string& str) {
	if (opt == "+")
		this->ClientInfo[idx] = str;
	else if (opt == "-")
		this->ClientInfo[idx] = "";
}

Triple<std::string, std::string, std::string> Client::parseMessage() {
    char buf[513] = {0};
	int len = recv(this->fd, buf, 512, 0);
	if (len <= 0) {
		throw Client::ClientException("Client disconnected");
	}
	buf[len] = '\0';
	std::string buffer(buf);
	if (buffer.size() == 512) {
		throw Client::ClientException("Message too long");
	}
	if ((buffer.find("\r\n")) == std::string::npos) {
		throw Client::ClientException("Message not complete");
	}
	std::string command, target, message;
    std::istringstream iss(buffer);
    iss >> command;
    std::getline(iss, target, ' ');
	
    std::getline(iss, message);
    if (!message.empty() && message[0] == ' ') {
        message.erase(0, 1);
    }
    return Triple<std::string, std::string, std::string>(command, target, message);
}

void Client::sendMessage(const std::string& message) const {
	send(fd, message.c_str(), message.size(), 0);
}

/* 
PRIVMSG #t hihi
PRIVMSG t0 hihi

TOPIC #t hihi

JOIN #t passwd
JOIN #t0

PART #t

KICK #t t0

INVITE #t t0

MODE #t +i/-i 	-> invite only
MODE #t +o/-o t0-> operator
MODE #t +k/-k 	-> password
MODE #t +l/-l 	-> limit 최대유저수
MODE #t +t/-t 	-> topic

MODE #t +iklt key 50 test
MODE #t +ik -lt key

MODE #t -ik -lt
MODE #t -iklt							최대유저수 오퍼레이터
<channel> {[+|-]|o|p|s|i|t|n|b|v} [<limit>] [<user>]
               [<ban mask>]




mode #채널이름 +/- ioklt 
// 

 */
