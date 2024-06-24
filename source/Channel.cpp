/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nahyulee <nahyulee@student.42seoul.kr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/08 19:10:06 by nahyulee          #+#    #+#             */
/*   Updated: 2024/06/25 04:57:58 by nahyulee         ###   ########.fr       */
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
	if (name.size() == 0 || name.size() > 9)
		return false;
	if (name[0] != '#')
		return false;
	for (size_t i = 1; i < name.size(); i++) {
		if (!isalnum(name[i]))
			return false;
	}
	if (name == "#root" || name == "#admin")
        return false;
	return true;
}

std::string Channel::checkDefaultInfo(int level) const {
    if (is(Channel::chatname).empty()) {
        return("You are not in the channels");
    }
    if (level >= 1) {
        if (is(Channel::topic).empty()) {
            return("Topic is not set");
        }
    }
    if (level >= 2) {
        if (is(Channel::limits).empty()) {
			return("Limits is not set");
		}
	}
	if (level >= 3) {
		if (is(Channel::passwd).empty()) {
			return("Password is not set");
		}
	}
	if (level == 4) {
		if (is(Channel::inviteOnly).empty()) {
			return("InviteOnly is not set");
		}
	}
	return "true";
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
		broadcast("Channel " + this->is(Channel::chatname) + " " + message + " " + str +" has been updated");
	} else if (opt == "-") {
		this->ChatInfo[idx].clear();
		broadcast("Channel " + this->is(Channel::chatname) + " " + message +" has been option removed");
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
		broadcast(client->is(Client::Nickname) + " has joined the channel (" + this->is(Channel::chatname) + ")");
		return ;
	}
	if (inout == "out" && this->clients.find(client) == this->clients.end()) {
		throw Channel::ChannelException("client not in the channel");
	} else if (inout == "out") {
		broadcast(client->is(Client::Nickname) + " has left the channel (" + this->is(Channel::chatname) + ")");
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
