/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nahyulee <nahyulee@student.42seoul.kr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/08 19:50:01 by nahyulee          #+#    #+#             */
/*   Updated: 2024/06/08 23:22:41 by nahyulee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "Client.hpp"

Client::Client() {}
Client::Client(const Client& copy) {}
Client& Client::operator=(const Client& copy) {}
Client::Client(int fd) : fd(fd) {}
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

std::string Client::is(enum e_info idx) const {
	return this->ClientInfo[idx];
}

void Client::set(enum e_info idx, const std::string opt, const std::string& str) {
	if (opt == "+")
		this->ClientInfo[idx] = str;
	else if (opt == "-")
		this->ClientInfo[idx] = NULL;
}

const char *Client::ClientException::what() const throw() {
	return "Client Exception: ";
}

Triple<std::string, std::string, std::string> parseMessage() {
    char buf[513] = {0};
	int len = recv(this->fd, buf, 512, 0);
	if (len <= 0) {
		throw Client::ClientException("Client disconnected");
	}
	if (buffer.size() == 512) {
		throw Client::ClientException("Message too long");
	}
	buf[len] = '\0';
	std::string buffer(buf);
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
/* 
PRIVMSG #t hihi
PRIVMSG t0 hihi

TOPIC #t hihi

JOIN #t passwd
JOIN #t0

PART #t

KICK #t t0

INVITE #t t0

MODE +i/-i #t 	-> invite only
MODE +o/-o #t t0-> operator
MODE +k/-k #t 	-> password
MODE +l/-l #t 	-> limit
MODE +t/-t #t 	-> topic
 */