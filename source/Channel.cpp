/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nahyulee <nahyulee@student.42seoul.kr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/08 19:10:06 by nahyulee          #+#    #+#             */
/*   Updated: 2024/06/15 18:33:43 by nahyulee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "Channel.hpp"
# include "Client.hpp"

Channel::Channel() {}
Channel::Channel(const Channel &copy) { *this = copy; }
Channel &Channel::operator=(const Channel &copy) { (void)copy; return *this; }
Channel::~Channel() {}
Channel::Channel(const std::string& name) {
	this->ChatInfo[chatname] = "#" + name;
	this->ChannelInit();
}

void Channel::ChannelInit() {
	this->ChatInfo[topic] = "";
	this->ChatInfo[limits] = "";
	this->ChatInfo[passwd] = "";
	this->ChatInfo[inviteOnly] = "";
}

std::string Channel::is(enum e_info idx) const {
	return this->ChatInfo[idx];
}

void Channel::set(enum e_info idx, const std::string opt, const std::string& str) {
	if (opt == "+")
		this->ChatInfo[idx] = str;
	else if (opt == "-")
		this->ChatInfo[idx].clear();
}

std::string Channel::ClientInOut(std::string inout, Client* client) throw(Channel::ChannelException) {
	if (inout == "in" && this->clients.find(client) != this->clients.end())
		throw Channel::ChannelException();
	this->clients.insert(client);
	return "Client added to channel";
	
	if (inout == "out" && this->clients.find(client) == this->clients.end())
		throw Channel::ChannelException();
	this->clients.erase(client);
	return "Client removed from channel";
}

void Channel::broadcast(const std::string& message, int except_fd) {
	for (std::set<Client*>::iterator it = clients.begin(); it != clients.end(); ++it) {
		if ((*it)->getFd() != except_fd) {
			send((*it)->getFd(), message.c_str(), message.size(), 0);
		}
	}
}
