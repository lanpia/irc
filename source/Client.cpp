/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nahyulee <nahyulee@student.42seoul.kr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/08 19:50:01 by nahyulee          #+#    #+#             */
/*   Updated: 2024/06/18 00:17:51 by nahyulee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "Client.hpp"

Client::Client() {}
Client::Client(const Client& copy) { *this = copy; }
Client& Client::operator=(const Client& copy) { (void) copy; return *this; }
Client::Client(int fd) : fd(fd) {
	this->ClientInfo[Nickname] = "";
	this->ClientInfo[Username] = "";
	this->ClientInfo[Chatname] = "";
	this->ClientInfo[Operator] = "";
}
Client::~Client() {}

bool Client::isValidNickname(const std::string& nickname) const {
	if (nickname.size() == 0 || nickname.size() > 9) {
		sendMessage("Invalid name");
		return false;
	}
	for (size_t i = 0; i < nickname.size(); i++) {
		if (!isalnum(nickname[i])) {
			sendMessage("Invalid name");
			return false;
		}
	}
	if (nickname == "root" || nickname == "admin") {
        sendMessage("Invalid name");
		return false;
	}
	return true;
}

int Client::getFd() const {
	return this->fd;
}

std::string Client::is(int idx) const {
	return this->ClientInfo[idx];
}

void Client::set(int idx, const std::string opt, const std::string& str) {
	std::string message;
	switch (idx) {
		case Nickname:
			message = "nickname";
			break;
		case Username:
			message = "username";
			break;
		case Chatname:
			message = "chatname";
			break;
		case Operator:
			message = "operator";
			break;
	}
	if (opt == "+") {
		this->ClientInfo[idx] = str;
		if (message != "chatname")
			sendMessage("clientinfomation of " + message + " is set for " + str);
	}
	else if (opt == "-") {
		this->ClientInfo[idx].clear();
		if (message != "chatname")
			sendMessage("clientinfomation of "+ message + " is cleared");
	}
}

bool Client::checkDefaultInfo(int level) const {
    if (is(Client::Nickname).empty() || is(Client::Username).empty()) {
        sendMessage("You have to set nickname and username");
        return false;
    }
    if (level >= 1) {
        if (is(Client::Chatname).empty()) {
            sendMessage("You are not in the channels");
            return false;
        }
    }
    if (level == 2) {
        if (is(Client::Operator) != "operator") {
			sendMessage("You are not operator");
            return false;
        }
    }
    return true;
}

void Client::sendMessage(const std::string& message) const {
    std::string messageWithNewline = message + "\r\n";
    send(fd, messageWithNewline.c_str(), messageWithNewline.size(), 0);
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
		sendMessage("Message too long");
		throw Client::ClientException("Message too long");
	}
	std::cout << "buffer: " << buffer << std::endl;
	if ((buffer.find("\r\n")) != std::string::npos) {
		sendMessage("Message not complete");
		throw Client::ClientException("Message not complete");
	}
	buffer.erase(buffer.find_last_not_of(" \n\r\t") + 1);
	std::string command, target, message;
    std::istringstream iss(buffer);
    iss >> command;
    std::getline(iss, target, ' ');
	iss >> target;
    std::getline(iss, message);
    if (!message.empty() && message[0] == ' ') {
        message.erase(0, 1);
    }
	std::cout << "command: " << command << std::endl;
	std::cout << "target: " << target << std::endl;
	std::cout << "message: " << message << std::endl;
    return Triple<std::string, std::string, std::string>(command, target, message);
}


Triple<int, std::string, std::string> Client::MODEparse(const std::string& message) {
    std::istringstream iss(message);
    char sign;
	char mode;
    std::string parameter;
    iss >> sign;
    if (sign != '+' && sign != '-') {
		sendMessage("Invalid MODE format");
        throw Client::ClientException("Invalid mode format");
    }
    iss >> mode;
	
    if (!iss.eof()) {
        iss >> parameter;
    }
    std::cout << "sign: " << sign << std::endl;
    std::cout << "mode: " << mode << std::endl;
    std::cout << "parameter: " << parameter << std::endl;
	enum e_info {operatorMode, topic, limits, passwd, inviteOnly};
	int option;
    switch (mode) {
        case 'o':
            option = operatorMode;
            break;
        case 't':
            option = topic;
            break;
        case 'l':
            option = limits;
            break;
        case 'k':
            option = passwd;
            break;
        case 'i':
            option = inviteOnly;
			parameter = "true";
            break;
        default:
            sendMessage("Unknown MODE");
			throw Client::ClientException("Unknown mode");
    }
	if (sign == '-') {
		parameter = "";
	}
    return Triple<int, std::string, std::string>(option, std::string(1, sign), parameter);
}

void Client::responseMessage(std::string code) const {
	std::map<std::string, std::string> _responseCode;
	_responseCode["001"] = ":Welcome to the Internet Relay Network ";
	_responseCode["002"] = ":Your host is ";
	_responseCode["003"] = ":This server was created ";
	_responseCode["253"] = ":unknown connection(s)";
	_responseCode["332"] = " :No topic is set";
	_responseCode["401"] = " :No such nick/channel";
	_responseCode["402"] = " :No such server";
	_responseCode["403"] = " :No such channel";
	_responseCode["405"] = " :You have joined too many channels";
	_responseCode["406"] = " :There was no such nickname";
	_responseCode["409"] = " :No origin specified";
	_responseCode["411"] = " :No recipient given";
	_responseCode["421"] = " :Unknown command";
	_responseCode["422"] = " :MOTD File is missing";
	_responseCode["432"] = " :Erroneous nickname";
	_responseCode["433"] = " :Nickname is already in use";
	_responseCode["442"] = " :You're not on that channel";
	_responseCode["443"] = " :is already on channel";
	_responseCode["461"] = " :Not enough parameters";
	_responseCode["464"] = " :Password incorrect";
	
	_responseCode["472"] = " :is unknown mode char to me";
	_responseCode["481"] = " :Permission Denied- You're not an IRC operator";
	_responseCode["483"] = " :You cant kill a server";
	_responseCode["501"] = " :Unknown MODE flag";
	_responseCode["502"] = " :Cannot change mode for other users";
	_responseCode["504"] = " :Cannot kill server";
	_responseCode["511"] = " :Channel doesn't exist";
	_responseCode["513"] = " :is already registered";
	
	std::map<std::string, std::string>::iterator it = _responseCode.find(code);
	if (it == _responseCode.end()) {
		sendMessage("\033[0;32mUnknown response code\033[0m");
	}
	sendMessage("\033[0;32mft_irc server <" + code + ">" + _responseCode[code] + "\033[0m");
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
