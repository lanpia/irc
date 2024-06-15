/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nahyulee <nahyulee@student.42seoul.kr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/08 19:58:05 by nahyulee          #+#    #+#             */
/*   Updated: 2024/06/16 01:17:08 by nahyulee         ###   ########.fr       */
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
	if (password.length() > 0) {
		clients[new_socket]->sendMessage("Enter password: ");
		
	}
	Triple<std::string, std::string, std::string> msg;
	msg = this->clients[new_socket]->parseMessage();
	if (msg.first == "PASS") {
		if (msg.third == password) {
				clients[new_socket]->sendMessage("Authentication successful!\n");
			} else {
				clients[new_socket]->sendMessage("Authentication failed!\n");
				close(new_socket);
				delete clients[new_socket];
				clients.erase(new_socket);
				return;
			}
	} else if (new_socket == 0) {
        close(new_socket);
        delete clients[new_socket];
        clients.erase(new_socket);
        std::cout << "Client disconnected: " << new_socket << std::endl;
    }
	std::cout << "New client connected: " << new_socket << std::endl;
}

void Server::sendMessage(const std::string& message) const {
	send(svrFd, message.c_str(), message.size(), 0);
}

void Server::disconnectClient(int client_fd) {
	std::cout << "Client disconnected: " << client_fd << std::endl;
	channels[clients[client_fd]->is(Client::Nickname)]->ClientInOut("out", clients[client_fd]);
	close(client_fd);
	delete clients[client_fd];
	clients.erase(client_fd);
}
void Server::handleClientMessage(int client_fd) {
    Triple<std::string, std::string, std::string> msg = this->clients[client_fd]->parseMessage();
    std::map<std::string, void (Server::*)(int, const std::string&, const std::string&)> _commands;
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
        clients[client_fd]->sendMessage("Unknown command: " + msg.second + " " + msg.third + "\r\n");
    }
}

bool Server::checkDefaultInfo(int level, int client_fd) {
    if (clients[client_fd]->is(Client::Nickname).empty() || clients[client_fd]->is(Client::Username).empty()) {
        clients[client_fd]->sendMessage("You have to set nickname and username\r\n");
        return false;
    }
    if (level >= 1) {
        if (clients[client_fd]->is(Client::InChannel).empty()) {
            clients[client_fd]->sendMessage("You are not in the channels\r\n");
            return false;
        }
    }
    if (level == 2) {
        if (clients[client_fd]->is(Client::Operator) == "operator") {
			clients[client_fd]->sendMessage("You are not operator\r\n");
            return false;
        }
    }
    return true;
}

void Server::handleNick(int client_fd, const std::string& target, const std::string& message) {
	for (std::map<int, Client*>::const_iterator it = clients.begin(); it != clients.end(); ++it) {
		if (it->second->is(Client::Nickname) == message) {
			clients[client_fd]->sendMessage("Nickname already in use\r\n");
			return;
		}
	}
	if (clients[client_fd]->isValidNickname(target)) {
		clients[client_fd]->sendMessage("Invalid nickname\r\n");
		return;
	}
	clients[client_fd]->set(Client::Nickname, "+", message);
	clients[client_fd]->sendMessage("NICK " + clients[client_fd]->is(Client::Nickname) + "\r\n");
}

void Server::handleUser(int client_fd, const std::string& target, const std::string& message) {
	// for (std::map<int, Client*>::const_iterator it = clients.begin(); it != clients.end(); ++it) {
	// 	if (it->second->is(Client::Username) == target) {
	// 		clients[client_fd]->sendMessage("Username already in use\r\n");
	// 		return;
	// 	}
	// }
	clients[client_fd]->set(Client::Username, "+", message);
	clients[client_fd]->sendMessage("USER " + clients[client_fd]->is(Client::Username) + "\r\n");
	(void)target;
}

void Server::handleJoin(int client_fd, const std::string& target, const std::string& message) {
	if (checkDefaultInfo(0, client_fd) == false) {
		return;
	}
	if (channels.find(target) == channels.end()) {
		channels[target] = new Channel(target);
		channels[target]->broadcast("Channel created: " + message + "\r\n", client_fd);
		clients[client_fd]->set(Client::Operator, "+", "operator");
		clients[client_fd]->set(Client::InChannel, "+", "true");
		clients[client_fd]->sendMessage("Now youer Channel operator\r\n");
		channels[target]->broadcast("you have to Setting TOPIC\r\n", client_fd);
	}
	if (channels[target]->is(Channel::inviteOnly) == "true" \
		&& static_cast<long unsigned int>(std::strtod(channels[target]->is(Channel::limits).c_str(), NULL)) == clients.size() \
		&& clients[client_fd]->is(Client::InChannel) == "true") {
		clients[client_fd]->sendMessage("JOIN fail\r\n");
		return ;
	}
	if (channels[target]->is(Channel::passwd).empty() == false && channels[target]->is(Channel::passwd) != message) {
		clients[client_fd]->sendMessage("JOIN fail\r\n");
		return ;
	}
	channels[target]->ClientInOut("in", clients[client_fd]);
	clients[client_fd]->sendMessage("JOIN " + channels[target]->is(Channel::chatname) + "\r\n");
	channels[target]->broadcast(clients[client_fd]->is(Client::Nickname) + " has joined the channes\r\n", client_fd);
}

void Server::handlePart(int client_fd, const std::string& target, const std::string& message) {
	if (channels.find(target) != channels.end()) {
		channels[target]->ClientInOut("out", clients[client_fd]);
		clients[client_fd]->sendMessage("PART " + target + "\r\n");
		channels[target]->broadcast(clients[client_fd]->is(Client::Nickname) + " has left the channes\r\n", client_fd);
	}
	(void)message;
}

void Server::handlePrivmsg(int client_fd, const std::string& target, const std::string& message) {
	if (target[0] == '#') {
		if (checkDefaultInfo(1, client_fd) == false) {
			return;
		}
		channels[target]->broadcast("PRIVMSG " + target + " :" + clients[client_fd]->is(Client::Nickname) + ": " + message + "\r\n", client_fd);
	} else {
		for (std::map<int, Client*>::iterator it = clients.begin(); it != clients.end(); ++it) {
			if (it->second->is(Client::Nickname) == target) {
				clients[client_fd]->sendMessage("PRIVMSG " + target + " :" + clients[client_fd]->is(Client::Nickname) + ": " + message + "\r\n");
			}
		}
	}
}

void Server::handleKick(int client_fd, const std::string& target, const std::string& message) {
	if (checkDefaultInfo(2, client_fd) == false) {
		return;
	}
	channels[target]->ClientInOut("out", clients[client_fd]);
	clients[client_fd]->sendMessage("KICK " + target + " " + message + "\r\n");
	channels[target]->broadcast(target + " has been kicked from the channes\r\n", client_fd);
}

void Server::handleInvite(int client_fd, const std::string& target, const std::string& message) {
	if (checkDefaultInfo(2, client_fd) == false) {
		return;
	}
	if (static_cast<long unsigned int>(std::strtod(channels[target]->is(Channel::limits).c_str(), NULL)) == clients.size() \
		&& clients[client_fd]->is(Client::InChannel) == "true") {
		channels[target]->ClientInOut("in", clients[client_fd]);
		clients[client_fd]->sendMessage("INVITE " + target + " " + message + "\r\n");
	}
}

void Server::handleTopic(int client_fd, const std::string& target, const std::string& message) {
	if (checkDefaultInfo(2, client_fd) == false) {
		return;
	}
	channels[target]->set(Channel::topic, "+", message);
	clients[client_fd]->sendMessage("TOPIC " + target + " :" + message + "\r\n");
	channels[target]->broadcast("TOPIC " + target + " :" + message + "\r\n", client_fd);
}

void Server::handleMode(int client_fd, const std::string& target, const std::string& message) {
	if (checkDefaultInfo(2, client_fd) == false) {
		return;
	}
	Triple<std::string, int, std::string> opt = modeparse(message);
	if (opt.second == 0)
		clients[client_fd]->set(Client::Operator, opt.first, opt.third);
	channels[target]->set(opt.second, opt.first, opt.third);
	clients[client_fd]->sendMessage("MODE " + target + " :" + message + "\r\n");
	channels[target]->broadcast("MODE " + target + " :" + message + "\r\n", client_fd);
}

void Server::handleQuit(int client_fd, const std::string& target, const std::string& message) {
	(void)target;
	(void)message;
	disconnectClient(client_fd);
}

Triple<std::string, int, std::string> Server::modeparse(std::string const &message) {
	Triple<std::string, int, std::string> opt;
	if (message[0] == '+') {
		opt.first = "+";
	} else if (message[0] == '-') {
		opt.first = "-";
	}
	if (message[1] == 'o') {
		opt.second = 0;
	} else if (message[1] == 't') {
		opt.second = 1;
	} else if (message[1] == 'l') {
		opt.second = 2;
	} else if (message[1] == 'k') {
		opt.second = 3;
	} else if (message[1] == 'i') {
		opt.second = 4;
	}
	opt.third = message.substr(2);
	return opt;
}