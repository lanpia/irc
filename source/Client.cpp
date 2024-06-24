/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nahyulee <nahyulee@student.42seoul.kr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/08 19:50:01 by nahyulee          #+#    #+#             */
/*   Updated: 2024/06/25 05:05:42 by nahyulee         ###   ########.fr       */
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

std::vector<std::string> Client::MODEcount(const std::string& message) {
    std::istringstream iss(message);
	std::string buffer;
	std::vector<std::string> token;
	std::string modes = "oklti";
	
	while (iss >> buffer) {
		token.push_back(buffer);
	}
	if (token.size() >= 2) {
		for (long unsigned int i = 1; i < token.size(); i++) {
			if (token[i][0] == '+' || token[i][0] == '-')
				throw Client::ClientException("Invalid MODE format 1");
		}
	}
	int first_token_len = 0;
	for (long unsigned int j = 0; j < token[0].size(); j++) {
		if (j == 0 && (token[0][j] == '+' || token[0][j] == '-'))
			continue;
		if (modes.find(token[0][j]) == std::string::npos)
			throw Client::ClientException("Invalid MODE format 2");
		if (std::count(token[0].begin(), token[0].end(), token[0][j]) > 1)  
			throw Client::ClientException("Invalid MODE format dup");
		first_token_len++;
	}
	if (first_token_len == 0)
		throw Client::ClientException("Invalid MODE format 3");
	return token;
}

Triple<int, std::string, std::string> Client::MODEparse(const char mode, std::vector<std::string>* token) {
	int option = 0;
	std::string modes = "oklti";
	std::string tok;
	
	
	switch (mode) {
		enum e_info {operatorMode, topic, limits, passwd, inviteOnly};
		case 'o': option = operatorMode; tok = token->back(); token->pop_back(); std::cout << "test o\n";break;
		case 't': option = topic; tok = token->back(); token->pop_back(); std::cout << "test t\n";break;
		case 'l':  option = limits; tok = token->back(); token->pop_back();
			std::cout << "tok : " << tok << std::endl;
			for (long unsigned int i = 0; i < tok.size(); i++) {
				if (!std::isdigit(tok[i]))
					throw Client::ClientException("Invalid MODE format 4");
			}
			std::cout << "test l\n";break;
		case 'k': option = passwd; tok = token->back(); token->pop_back(); 
			std::cout << "tok : " << tok << std::endl;
			std::cout << "test k\n"; break;
		case 'i': option = inviteOnly; tok = "true"; std::cout << "test i\n"; break;
		default:
			sendMessage("Unknown MODE");
			throw Client::ClientException("Unknown mode");
	}

	return (Triple<int, std::string, std::string>(option, std::string(1, mode), tok));
}

// void Client::responseMessage(std::string code) const {
// 	std::map<std::string, std::string> _responseCode;
// 	_responseCode["001"] = ":Welcome to the Internet Relay Network ";
// 	_responseCode["002"] = ":Your host is ";
// 	_responseCode["003"] = ":This server was created ";
// 	_responseCode["253"] = ":unknown connection(s)";
// 	_responseCode["332"] = " :No topic is set";
// 	_responseCode["401"] = " :No such nick/channel";
// 	_responseCode["402"] = " :No such server";
// 	_responseCode["403"] = " :No such channel";
// 	_responseCode["405"] = " :You have joined too many channels";
// 	_responseCode["406"] = " :There was no such nickname";
// 	_responseCode["409"] = " :No origin specified";
// 	_responseCode["411"] = " :No recipient given";
// 	_responseCode["421"] = " :Unknown command";
// 	_responseCode["422"] = " :MOTD File is missing";
// 	_responseCode["432"] = " :Erroneous nickname";
// 	_responseCode["433"] = " :Nickname is already in use";
// 	_responseCode["442"] = " :You're not on that channel";
// 	_responseCode["443"] = " :is already on channel";
// 	_responseCode["461"] = " :Not enough parameters";
// 	_responseCode["464"] = " :Password incorrect";
	
// 	_responseCode["472"] = " :is unknown mode char to me";
// 	_responseCode["481"] = " :Permission Denied- You're not an IRC operator";
// 	_responseCode["483"] = " :You cant kill a server";
// 	_responseCode["501"] = " :Unknown MODE flag";
// 	_responseCode["502"] = " :Cannot change mode for other users";
// 	_responseCode["504"] = " :Cannot kill server";
// 	_responseCode["511"] = " :Channel doesn't exist";
// 	_responseCode["513"] = " :is already registered";
	
// 	std::map<std::string, std::string>::iterator it = _responseCode.find(code);
// 	if (it == _responseCode.end()) {
// 		sendMessage("\033[0;32mUnknown response code\033[0m");
// 	}
// 	sendMessage("\033[0;32mft_irc server <" + code + ">" + _responseCode[code] + "\033[0m");
// }
