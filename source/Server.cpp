/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nahyulee <nahyulee@student.42seoul.kr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/08 19:58:05 by nahyulee          #+#    #+#             */
/*   Updated: 2024/06/24 23:09:17 by nahyulee         ###   ########.fr       */
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
	if (password.length() > 0)
		clients[new_socket]->sendMessage("Enter password");
}

void Server::disconnectClient(int client_fd) {
	std::cout << "Client disconnected: " << client_fd << std::endl;
	close(client_fd);
	delete clients[client_fd];
	clients.erase(client_fd);
}

void Server::handleClientMessage(int client_fd) {
	Triple<std::string, std::string, std::string> msg = this->clients[client_fd]->parseMessage();
	std::map<std::string, void (Server::*)(int, const std::string&, const std::string&)> _commands;
	_commands["PASS"] = &Server::handlePass;
	_commands["USER"] = &Server::handleUser;
	_commands["NICK"] = &Server::handleNick;
	_commands["JOIN"] = &Server::handleJoin;
	_commands["PART"] = &Server::handlePart;
	_commands["PRIVMSG"] = &Server::handlePrivmsg;
	_commands["KICK"] = &Server::handleKick;
	_commands["INVITE"] = &Server::handleInvite;
	_commands["TOPIC"] = &Server::handleTopic;
	_commands["MODE"] = &Server::handleMode;
	_commands["QUIT"] = &Server::handleQuit;

	std::map<std::string, void (Server::*)(int, const std::string&, const std::string&)>::iterator it = _commands.find(msg.first);
	if (it != _commands.end()) {
		(this->*(it->second))(client_fd, msg.second, msg.third);
	} else {
		clients[client_fd]->sendMessage("Unknown command: " + msg.first + " " + msg.second + " " + msg.second);
	}
}

int Server::findClientFd(const std::string& target) {
	for (std::map<int, Client*>::iterator it = clients.begin(); it != clients.end(); ++it) {
		if (it->second->is(Client::Nickname) == target) {
			return it->first;
		}
	}
	return -1;
}

void Server::handlePass(int client_fd, const std::string& target, const std::string& message) {
	if (target == password) {
		clients[client_fd]->sendMessage("Authentication successful!\nNow you have to set the Nickname, Username");
	} else {
		clients[client_fd]->sendMessage("Authentication failed!");
		disconnectClient(client_fd);
	} 
	if (client_fd == 0) {
		disconnectClient(client_fd);
		std::cout << "Client disconnected: " << client_fd << std::endl;
	}
	(void)message;
}

void Server::handleNick(int client_fd, const std::string& target, const std::string& message) {
	for (std::map<int, Client*>::const_iterator it = clients.begin(); it != clients.end(); ++it) {
		if (it->second->is(Client::Nickname) == target) {
			clients[client_fd]->sendMessage("Nickname already in use");
			return;
		}
	}
	if (clients[client_fd]->isValidNickname(target) == false) {
		return;
	}
	clients[client_fd]->set(Client::Nickname, "+", target);
	(void)message;
}

void Server::handleUser(int client_fd, const std::string& target, const std::string& message) {
	if (clients[client_fd]->isValidNickname(target) == false) {
		return;
	}
	clients[client_fd]->set(Client::Username, "+", target + " " + message);
}

void Server::handleJoin(int client_fd, const std::string& target, const std::string& message) {
	if (clients[client_fd]->checkDefaultInfo(0) == false) {
		return;
	}
	if (channels.find(target) == channels.end()) {
		channels[target] = new Channel(target);
		channels[target]->setFirstOperator(clients[client_fd]);
		channels[target]->broadcast("Channel created: " + target, client_fd);
		clients[client_fd]->set(Client::Operator, "+", "operator");
	}
	if (channels[target]->is(Channel::inviteOnly) == "true" \
		&& static_cast<long unsigned int>(std::strtod(channels[target]->is(Channel::limits).c_str(), NULL)) == clients.size() \
		&& clients[client_fd]->is(Client::Chatname) == "true") {
		clients[client_fd]->sendMessage("JOIN fail");
		return ;
	}
	if (channels[target]->is(Channel::passwd).empty() == false && channels[target]->is(Channel::passwd) != message) {
		clients[client_fd]->sendMessage("JOIN fail");
		return ;
	}
	if (channels[target]->is(Channel::passwd).empty() == false && channels[target]->is(Channel::passwd) != message) {
		clients[client_fd]->sendMessage("JOIN fail, passwd error");
		return ;
	} else if (channels[target]->is(Channel::passwd).empty() == false && channels[target]->is(Channel::passwd) == message) {
		clients[client_fd]->sendMessage("PASSWORD OK");
	}
	clients[client_fd]->set(Client::Chatname, "+", target);
	channels[target]->ClientInOut("in", clients[client_fd]);
}

void Server::handlePart(int client_fd, const std::string& target, const std::string& message) {
	if (channels.find(target) != channels.end() && clients[client_fd] != channels[target]->getFirstOperator()) {
		channels[target]->ClientInOut("out", clients[client_fd]);
		clients[client_fd]->set(Client::Operator, "-", "");
		clients[client_fd]->set(Client::Chatname, "-", "");
	} else if (channels.find(target) != channels.end() && clients[client_fd] == channels[target]->getFirstOperator()) {
		channels[target]->broadcast("Channel deleted: " + target, client_fd);
		for (std::set<Client*>::iterator it = channels[target]->getClients().begin(); it != channels[target]->getClients().end(); ++it) {
			(*it)->set(Client::Chatname, "-", "");
			(*it)->set(Client::Operator, "-", "");
			channels[target]->ClientInOut("out", *it);
		}
		channels[target]->setFirstOperator(NULL);
		delete channels[target];
	}
	(void)message;
}

void Server::handlePrivmsg(int client_fd, const std::string& target, const std::string& message) {
	if (target[0] == '#') {
		if (clients[client_fd]->checkDefaultInfo(1) == false) {
			return;
		}
		if (channels.find(target) == channels.end()) {
			clients[client_fd]->sendMessage("Channel not found");
			return;
		}
		channels[target]->broadcast("PRIVMSG " + target + " :" + clients[client_fd]->is(Client::Nickname) + ": " + message, client_fd);
	} else {
		for (std::map<int, Client*>::iterator it = clients.begin(); it != clients.end(); ++it) {
			if (it->second->is(Client::Nickname) == target) {
				clients[it->second->getFd()]->sendMessage("PRIVMSG " + target + " :" + clients[client_fd]->is(Client::Nickname) + ": " + message);
			}
		}
	}
}

void Server::handleKick(int client_fd, const std::string& target, const std::string& message) {
	if (clients[client_fd]->checkDefaultInfo(2) == false) {
		return;
	}
	int target_fd = findClientFd(message);
	if (target_fd == -1) {
		clients[client_fd]->sendMessage("Client not found");
		return;
	} else if (client_fd == target_fd) {
		clients[client_fd]->sendMessage("You can't kick yourself");
		return;
	} else {
		channels[target]->ClientInOut("out", clients[client_fd]);
		clients[target_fd]->set(Client::Operator, "-", "");
		clients[target_fd]->set(Client::Chatname, "-", "");
	}
}

void Server::handleInvite(int client_fd, const std::string& target, const std::string& message) {
	if (clients[client_fd]->checkDefaultInfo(2) == false) {
		return;
	}
	int target_fd = findClientFd(message);
	if (target_fd == -1) {
		clients[client_fd]->sendMessage("Client not found");
		return;
	} else if (client_fd == target_fd) {
		clients[client_fd]->sendMessage("You can't Invite yourself");
		return;
	} else {
		if (channels[target]->is(Channel::passwd).empty() == false && channels[target]->is(Channel::passwd) != message) {
			clients[client_fd]->sendMessage("Invite fail");
			return ;
		}
		if (channels[target]->is(Channel::passwd).empty() == false && channels[target]->is(Channel::passwd) != message) {
			clients[client_fd]->sendMessage("Invite fail, passwd error");
			return ;
		} else if (channels[target]->is(Channel::passwd).empty() == false && channels[target]->is(Channel::passwd) == message) {
			clients[client_fd]->sendMessage("PASSWORD OK");
		}
		if (static_cast<long unsigned int>(std::strtod(channels[target]->is(Channel::limits).c_str(), NULL)) == clients.size() \
			&& clients[target_fd]->is(Client::Chatname).empty()) {
			clients[target_fd]->set(Client::Chatname, "+", target);
			clients[target_fd]->sendMessage("INVITE " + target + " " + message);
			channels[target]->ClientInOut("in", clients[target_fd]);
		}
	}
}

void Server::handleTopic(int client_fd, const std::string& target, const std::string& message) {
	if (message.empty() == true) {
		clients[client_fd]->sendMessage(clients[client_fd]->is(Client::Chatname) + " : " + channels[target]->is(Channel::topic));
		return;
	} else {
		if (clients[client_fd]->checkDefaultInfo(2) == false) {
			return;
		}
		channels[target]->set(Channel::topic, "+", message);
		channels[target]->broadcast("TOPIC " + target + " :" + message, client_fd);
	}
}

void Server::handleMode(int client_fd, const std::string& target, const std::string& message) {
	if (clients[client_fd]->checkDefaultInfo(2) == false) {
		return;
	} else if (clients[client_fd]->is(Client::Chatname) != target) {
		clients[client_fd]->sendMessage("Channelname Error");
		return ;
	}
	
	std::cout << "\033[0;32m";
	std::cout << "target: " << target << std::endl;
	std::cout << "message: " << message << std::endl;
	
	std::vector<std::string> token = clients[client_fd]->MODEcount(message);


	char sign = *(token.front().begin());
	if (sign != '+' && sign != '-') {
		throw std::runtime_error("sign error");
	}
	token.front().erase(token.front().begin());

	std::cout << "sign: " << sign << std::endl;

	do {

		
		for (std::vector<std::string>::iterator it = token.begin(); it != token.end(); ++it) {
			std::cout << "token: " << *it << std::endl;
		}
		std::cout << "token size " << token.size() << std::endl;
	
		char mode = *(token.front().end() - 1);
		std::cout << "mode: " << mode << std::endl;
		token.front().erase(token.front().end() - 1);
		Triple<int, std::string, std::string> opt = clients[client_fd]->MODEparse(mode, &token);
		opt.second = sign;
		
		std::cout << "opt.first: " << opt.first << " opt.second: " << opt.second <<  " opt.third: " << opt.third << std::endl;

		if (opt.first == 0) {
			if (findClientFd(opt.third) != channels[target]->getFirstOperator()->getFd()) {
				clients[client_fd]->sendMessage("You are the first operator");
				return ;
			}
			clients[findClientFd(opt.third)]->set(Client::Operator, opt.second, "operator");
		} else {
			channels[target]->set(opt.first, opt.second, opt.third);
		}
	} while (!token.front().empty()) ;

	std::cout << "\033[0m" << std::endl;

	channels[target]->broadcast("MODE " + target + " :" + message, client_fd);
}

void Server::handleQuit(int client_fd, const std::string& target, const std::string& message) {
	(void)target;
	(void)message;
	clients[client_fd]->sendMessage("QUIT the server");
	disconnectClient(client_fd);
}
