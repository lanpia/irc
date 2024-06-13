/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nahyulee <nahyulee@student.42seoul.kr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/08 19:58:05 by nahyulee          #+#    #+#             */
/*   Updated: 2024/06/14 00:55:16 by nahyulee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

Server::Server() {}
Server::Server(const Server& copy) { *this = copy; }
Server& Server::operator=(const Server& copy) { (void)copy; return *this; }

Server::~Server() {
	if (svrFd != -1) {
		close(svrFd);
	}
	for (std::map<int, Client*>::iterator it = clients.begin(); it != clients.end(); ++it) {
		delete it->second;
		close(it->first);
	}
	for (std::map<std::string, Channel*>::iterator it = channels.begin(); it != channels.end(); ++it) {
		delete it->second;
	}
}

Server::Server(int port, const std::string& password) : port(port), password(password) {
	svrFd = socket(AF_INET, SOCK_STREAM, 0);
	if (svrFd == -1) {
		throw ServerException("Failed to create socket");
	}
	int opt = 1;
	if (setsockopt(svrFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
		throw std::runtime_error("Set socket options failed");
	}
	_address.sin_family = AF_INET;
	_address.sin_addr.s_addr = INADDR_ANY;
	_address.sin_port = htons(port);
	if (bind(svrFd, (struct sockaddr*)&_address, sizeof(_address)) < 0) {
		throw std::runtime_error("Bind failed");
	}
	if (listen(svrFd, 10) < 0) {
		throw std::runtime_error("Listen failed");
	}
	if (fcntl(svrFd, F_SETFL, O_NONBLOCK) < 0) {
		throw std::runtime_error("Set non-blocking failed");
	}

}
void Server::ServerRun() {
	struct pollfd fds[255];
	int nfds = 1;
	fds[0].fd = svrFd;
	fds[0].events = POLLIN;

	while (true) {
		int ret = poll(fds, nfds, -1);
		if (ret < 0)
			throw std::runtime_error("Polling error");
		for (int i = 0; i < nfds; ++i) {
			if (!(fds[i].revents & POLLIN))
				continue;
			if (fds[i].fd == svrFd) {
				acceptNewClient();
			} else {
				handleClientMessage(fds[i].fd);
			}
		}
		nfds = 1;
		fds[0].fd = svrFd;
		fds[0].events = POLLIN;
		for (std::map<int, Client*>::iterator it = clients.begin(); it != clients.end(); ++it) {
			fds[nfds].fd = it->first;
			fds[nfds].events = POLLIN;
			nfds++;
		}
	}
}
void Server::acceptNewClient() {
	int new_socket = accept(svrFd, NULL, NULL);
	if (new_socket < 0) {
		return;
	}

	fcntl(new_socket, F_SETFL, O_NONBLOCK);
	clients[new_socket] = new Client(new_socket);

	std::cout << "New client connected: " << new_socket << std::endl;
}


void Server::broadcast(const std::string& message, int except_fd) {
	for (std::map<int, Client*>::iterator it = clients.begin(); it != clients.end(); ++it) {
		if (it->first != except_fd) {
			send(it->first, message.c_str(), message.size(), 0);
		}
	}
}

void Server::sendMessage(const std::string& message) const {
	send(svrFd, message.c_str(), message.size(), 0);
}
\
void Server::disconnectClient(int client_fd) {
	std::cout << "Client disconnected: " << client_fd << std::endl;
	for (std::map<std::string, Channel*>::iterator it = channels.begin(); it != channels.end(); ++it) {
		it->second->clients[client_fd] = false;
	}
	close(client_fd);
	delete clients[client_fd];
	clients.erase(client_fd);
}

void Server::handleClientMessage(int client_fd) {
	Triple<std::string, std::string, std::string> msg;
	// Triple<std::string, std::string, std::string>(command, target, message);
	msg = this->clients[client_fd]->parseMessage();
	std::map<std::string, void (Server::*)(int, const std::string&)> _commands;
	_commands["NICK"] = &Server::handleNick;
	_commands["USER"] = &Server::handleUser;
	_commands["JOIN"] = &Server::handleJoin;
	_commands["PART"] = &Server::handlePart;
	_commands["PRIVMSG"] = &Server::handlePrivmsg;
	_commands["KICK"] = &Server::handleKick;
	_commands["INVITE"] = &Server::handleInvite;
	_commands["TOPIC"] = &Server::handleTopic;
	_commands["MODE"] = &Server::handleMode;
	_commands["QUIT"] = &Server::handleQuit;
	std::map<std::string, void (Server::*)(int, const std::string&)>::iterator it = msg.first;
	if (it != _commands.end()) {
		(this->*(it->second))(client_fd, msg.second, msg.third);
	} else {
		clients[client_fd]send("Unknown command: " + command + "\r\n");
	}
}

static bool isAlreadyUsed(const std::map<int, Client*>& clients, std::string (Client::*func)() const, const std::string& params) {
    for (std::map<int, Client*>::const_iterator it = clients.begin(); it != clients.end(); ++it) {
        if ((it->second->*func)() == params) {
            return true;
        }
    }
    return false;
}

void Server::handleNick(int client_fd, const std::string& target, const std::string& message) {
	if (isAlreadyUsed(_clients, &Client::getNickname, params)) {
		clients[client_fd]->sendMessage("Nickname already in use\r\n");
		return;
	}
	if (clients[client_fd]->isValidNickname(params) == false) {
		clients[client_fd]->sendMessage("Invalid nickname\r\n");
		return;
	}
	clients[client_fd]->set(Nickname, "+", message);
	clients[client_fd]->sendMessage("NICK " + _clients[client_fd]->is(nickname) + "\r\n");
}

void Server::handleUser(int client_fd, const std::string& target, const std::string& message) {
	if (isAlreadyUsed(_clients, &Client::getUsername, params)) {
		_clients[client_fd]->sendMessage("Username already in use\r\n");
		return;
	}
	_clients[client_fd]->set(Username, '+', message);
	_clients[client_fd]->sendMessage("USER " + _clients[client_fd]->getUsername() + "\r\n");
}

void Server::handleJoin(int client_fd, const std::string& target, const std::string& message) {
	if (_clients[client_fd]->getNickname().empty() || _clients[client_fd]->getUsername().empty()) {
		_clients[client_fd]->sendMessage("You have to setting Username, Nickname\r\n");
		return;
	}
	if (_channels.find(channelName) == _channels.end()) {
		_channels[channelName] = new Channel(channelName);
		_channels[channelName]->broadcast("Channel created: " + channelName + "\r\n");
		clients[client_fd]->set(Operator, '+', "operator");
		_clients[client_fd]->sendMessage("Now youer Channel operator\r\n");
		_channels[channelName]->broadcast("you have to Setting TOPIC\r\n");
	}
	// if (_channels[channelName]->isInviteonly() == true) {
	// 	_clients[client_fd]->sendMessage("JOIN fail\r\n");
	// 	return ;
	// }
	// if(_channels[channelName]->getLimit() != 0 && _channels[channelName]->getLimit() <= _channels[channelName]->getClients().size()) {
	// 	_clients[client_fd]->sendMessage("Channel is full\r\n");
	// 	return ;
	// }
	if (_channels[channelName]->checkMode(params) != "Success") {
		// exception(std::string )
		_clients[client_fd]->sendMessage(_channels[channelName]->checkMode(params) + "\r\n");
		return;
	}
	_channels[channelName]->addClient(_clients[client_fd]);
	_clients[client_fd]->sendMessage("JOIN " + channelName + "\r\n");
	_channels[channelName]->broadcast(_clients[client_fd]->getNickname() + " has joined the channel\r\n", client_fd);
	if (_channels[channelName]->getTopic().empty() == false)
		_channels[channelName]->broadcast("TOPIC " + channelName + " :" + _channels[channelName]->getTopic() + "\r\n");
}

void Server::handlePart(int client_fd, const std::string& target, const std::string& message) {
	std::string channelName = params;
	if (_channels.find(channelName) != _channels.end()) {
		_channels[channelName]->removeClient(_clients[client_fd]);
		_clients[client_fd]->sendMessage("PART " + channelName + "\r\n");
		_channels[channelName]->broadcast(_clients[client_fd]->getNickname() + " has left the channel\r\n", client_fd);
	}
}

void Server::handlePrivmsg(int client_fd, const std::string& target, const std::string& message) {
	size_t pos = params.find(" ");
	std::string target = params.substr(0, pos);
	std::string msg = params.substr(pos + 1);
	if (target[0] == '#') {
		// if (!isClientInChannel(*_clients[client_fd], target)) {
		if (!_channels[target]->isClientInChannel(_clients[client_fd])) {
			_clients[client_fd]->sendMessage("You are not in the channel\r\n");
			return;
		}
		// 채널 메시지
		_channels[target]->broadcast("PRIVMSG " + target + " :" + _clients[client_fd]->getNickname() + ": " + msg + "\r\n", client_fd);
	} else {
		// 개인 메시지
		for (std::map<int, Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
			if (it->second->getNickname() == target) {
				_clients[client_fd]->sendMessage("PRIVMSG " + target + " :" + _clients[client_fd]->getNickname() + ": " + msg + "\r\n");
			}
		}
	}
}

void Server::handleKick(int client_fd, const std::string& target, const std::string& message) {
	size_t pos = params.find(" ");
	std::string channelName = params.substr(0, pos);
	std::string target = params.substr(pos + 1);
	// if (!isClientInChannel(*_clients[client_fd], target)) {
	if (!_channels[channelName]->isClientInChannel(_clients[client_fd])) {
		_clients[client_fd]->sendMessage("You are not in the channel\r\n");
		return;
	}
	if (_channels.find(channelName) != _channels.end() && _channels[channelName]->isOperator(_clients[client_fd])) {
		// _channels[channelName]->kickClient(target);
		_channels[channelName]->removeClient(_clients[client_fd]);
		_clients[client_fd]->sendMessage("KICK " + channelName + " " + target + "\r\n");
		_channels[channelName]->broadcast(target + " has been kicked from the channel\r\n");
	}
}

void Server::handleInvite(int client_fd, const std::string& target, const std::string& message) {
	size_t pos = params.find(" ");
	std::string target = params.substr(0, pos);
	std::string channelName = params.substr(pos + 1);
	if (!_channels[channelName]->isClientInChannel(_clients[client_fd])) {
		_clients[client_fd]->sendMessage("You are not in the channel\r\n");
		return;
	}
	if (_channels.find(channelName) != _channels.end() && _channels[channelName]->isOperator(_clients[client_fd])) {
		_channels[channelName]->inviteClient(target);
		_clients[client_fd]->sendMessage("INVITE " + target + " " + channelName + "\r\n");
	}
}

void Server::handleTopic(int client_fd, const std::string& target, const std::string& message) {
	size_t pos = params.find(" ");
	std::string channelName = params.substr(0, pos);
	std::string topic = params.substr(pos + 1);
	if (!_channels[channelName]->isClientInChannel(_clients[client_fd])) {
		_clients[client_fd]->sendMessage("You are not in the channel\r\n");
		return;
	}
	if (_channels.find(channelName) != _channels.end() && _channels[channelName]->isOperator(_clients[client_fd])) {
		_channels[channelName]->setTopic(topic);
		_clients[client_fd]->sendMessage("TOPIC " + channelName + " :" + topic + "\r\n");
		_channels[channelName]->broadcast("TOPIC " + channelName + " :" + topic + "\r\n");
	}
}

void Server::handleMode(int client_fd, const std::string& target, const std::string& message) {
	size_t pos = params.find(" ");
	std::string channelName = params.substr(0, pos);
	std::string mode = params.substr(pos + 1);
	std::cout << pos << channelName << mode << std::endl;
	if (!_channels[channelName]->isClientInChannel(_clients[client_fd])) {
		_clients[client_fd]->sendMessage("You are not in the channel\r\n");
		return;
	}
	// if (_channels[channelName]->checkMode(params) != "Success")
	if (_channels.find(channelName) != _channels.end() && _channels[channelName]->isOperator(_clients[client_fd])) {
		_channels[channelName]->setMode(mode);
		_channels[channelName]->ChangeMode(mode);
		_clients[client_fd]->sendMessage("MODE " + channelName + " " + mode + "\r\n");
		_channels[channelName]->broadcast("MODE " + channelName + " " + mode + "\r\n");
	}
}

void Server::handleQuit(int client_fd, const std::string& params) {
	(void)params;
	disconnectClient(client_fd);
}
