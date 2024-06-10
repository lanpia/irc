/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nahyulee <nahyulee@student.42seoul.kr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/08 19:10:06 by nahyulee          #+#    #+#             */
/*   Updated: 2024/06/10 13:19:30 by nahyulee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "Include/Channel.hpp"

Channel::Channel() {}
Channel::~Channel() {}
Channel::Channel(const std::string& name) {
	this->ChatInfo[chatname] = name;
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
	if (inout == "in" && this->_clients.find(client) != this->_clients.end())
		throw Channel::ChannelException();
	this->_clients.insert(client);
	return "Client added to channel";
	
	if (inout == "out" && this->_clients.find(client) == this->_clients.end())
		throw Channel::ChannelException();
	this->_clients.erase(client);
	return "Client removed from channel";
}

std::string Channel::OperatorInOut(std::string inout, Client* client)  throw(Channel::ChannelException) {
	if (inout == "in" && this->_clients.find(client) != this->_clients.end())
		throw Channel::ChannelException();
	this->_clients.insert(client);
	return "Client added to channel";
	
	if (inout == "out" && this->_clients.find(client) == this->_clients.end())
		throw Channel::ChannelException();
	this->_clients.erase(client);
	return "Client removed from channel";
}
