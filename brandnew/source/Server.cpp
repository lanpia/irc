/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nahyulee <nahyulee@student.42seoul.kr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/08 19:58:05 by nahyulee          #+#    #+#             */
/*   Updated: 2024/06/10 13:23:14 by nahyulee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Include/Server.hpp"

Server::Server() {}
Server::~Server() {
	if (svrFd != -1) {
		close(svrFd);
	}
	for (std::vector<Client*>::iterator it = clients.begin(); it != clients.end(); ++it) {
		delete it->second;
		close(it->first);
	}
	for (std::map<std::string, Channel*>::iterator it = _channels.begin(); it != _channels.end(); ++it) {
		delete it->second;
	}
}
Server::Server(const Server& copy) {}
Server& Server::operator=(const Server& copy) {}
Server::Server(int port, const std::string& password) : port(port), password(password) {
	fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd == -1) {
		throw ServerException("Failed to create socket");
	}
	_addr.sin_family = AF_INET;
	_addr.sin_addr.s_addr = INADDR_ANY;
	_addr.sin_port = htons(port);
	if (bind(_fd, (struct sockaddr*)&_addr, sizeof(_addr)) == -1) {
		throw ServerException("Failed to bind socket");
	}
	if (listen(_fd, 10) == -1) {
		throw ServerException("Failed to listen on socket");
	}
}

void Channel::broadcast(const std::string& message, int except_fd) {
	for (std::set<Client*>::iterator it = clients.begin(); it != clients.end(); ++it) {
		if ((*it)->getFd() != except_fd) {
			(*it)->sendMessage(message);
		}
	}
}

void Client::sendMessage(const std::string& message) const {
	send(_fd, message.c_str(), message.size(), 0);
}