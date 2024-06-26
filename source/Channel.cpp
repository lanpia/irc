/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nahyulee <nahyulee@student.42seoul.kr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/08 19:10:06 by nahyulee          #+#    #+#             */
/*   Updated: 2024/06/26 23:03:29 by nahyulee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "Channel.hpp"
# include "Client.hpp"

Channel::Channel() {}
Channel::Channel(const Channel &copy) { *this = copy; }
Channel &Channel::operator=(const Channel &copy) { (void)copy; return *this; }
Channel::~Channel() {}
Channel::Channel(const std::string& name) {
	if (!isValidChatname(name))
		throw Channel::ChannelException("channel name is invalid");
	this->ChatInfo[chatname] = name;
	this->ChatInfo[topic] = "";
	this->ChatInfo[limits] = "";
	this->ChatInfo[passwd] = "";
	this->ChatInfo[inviteOnly] = "";
}

bool Channel::isValidChatname(const std::string& name) const {
	if (name.size() == 0 || name.size() > 9) {
		throw Channel::ChannelException("channel name is too long");
		return false;
	}
	if (name[0] != '#') {
		throw Channel::ChannelException("channel name have to start '#'");
		return false;
	}
	for (size_t i = 1; i < name.size(); i++) {
		if (!isalnum(name[i])) {
			throw Channel::ChannelException("channel name is invalid");
			return false;
		}
	}
	if (name == "#root" || name == "#admin") {
		throw Channel::ChannelException("channel name is invalid");
		return false;
	}
	return true;
}

std::string Channel::is(int idx) const {
	return this->ChatInfo[idx];
}

void Channel::set(int idx, const std::string opt, const std::string& str) {
	std::string message;
	switch (idx) {
		case chatname:
			message = "chatname";
			break;
		case topic:
			message = "topic";
			break;
		case limits:
			message = "limits";
			break;
		case passwd:
			message = "passwd";
			break;
	}
	if (opt == "+") {
		this->ChatInfo[idx] = str;
		broadcast("Channel " + this->is(Channel::chatname) + " " + message + " " + str +" has been updated", ANSI_BLUE);
	} else if (opt == "-") {
		this->ChatInfo[idx].clear();
		broadcast("Channel " + this->is(Channel::chatname) + " " + message +" has been option removed", ANSI_BLUE);
	}
}

std::set<Client*> Channel::getClients() {
	return this->clients;
}

Client* Channel::getFirstOperator() {
	return this->firstOperator;
}

void Channel::setFirstOperator(Client *op) {
	this->firstOperator = op;
}

void Channel::ClientInOut(std::string inout, Client* client) throw(Channel::ChannelException) {
	if (inout == "in" && this->clients.find(client) != this->clients.end()) {
		throw Channel::ChannelException("client already in the channel");
	} else if (inout == "in") {
		this->clients.insert(client);
		broadcast(client->is(Client::Nickname) + " has joined the channel (" + this->is(Channel::chatname) + ")", ANSI_BLUE);
		return ;
	}
	if (inout == "out" && this->clients.find(client) == this->clients.end()) {
		throw Channel::ChannelException("client not in the channel");
	} else if (inout == "out") {
		broadcast(client->is(Client::Nickname) + " has left the channel (" + this->is(Channel::chatname) + ")", ANSI_BLUE);
		this->clients.erase(client);
		return ;
	}
}

void Channel::broadcast(const std::string& message, int except_fd) {
	for (std::set<Client*>::iterator it = clients.begin(); it != clients.end(); ++it) {
		if ((*it)->getFd() != except_fd) {
			std::string messageWithNewline = message + "\r\n";
			send((*it)->getFd(), messageWithNewline.c_str(), messageWithNewline.size(), 0);
		}
	}
}

void Channel::broadcast(const std::string& message) {
	for (std::set<Client*>::iterator it = clients.begin(); it != clients.end(); ++it) {
		std::string messageWithNewline = message + "\r\n";
		send((*it)->getFd(), messageWithNewline.c_str(), messageWithNewline.size(), 0);
	}
}

void Channel::broadcast(const std::string& message, std::string color) {
	for (std::set<Client*>::iterator it = clients.begin(); it != clients.end(); ++it) {
		std::string messageWithNewline = color + message + "\r\n" + "\033[0m";
		send((*it)->getFd(), messageWithNewline.c_str(), messageWithNewline.size(), 0);
	}
}