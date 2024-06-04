#include "Server.hpp"

#include <arpa/inet.h>
#include <fcntl.h>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <sstream>

Server::Server(int port, const std::string& password)
	: _port(port), _password(password), _server_fd(-1) {
	_commands = {
		{"NICK", &Server::handleNick},
		{"USER", &Server::handleUser},
		{"JOIN", &Server::handleJoin},
		{"PART", &Server::handlePart},
		{"PRIVMSG", &Server::handlePrivmsg},
		{"KICK", &Server::handleKick},
		{"INVITE", &Server::handleInvite},
		{"TOPIC", &Server::handleTopic},
		{"MODE", &Server::handleMode},
		{"QUIT", &Server::handleQuit}};
}

Server::~Server() {
	if (_server_fd != -1) {
		close(_server_fd);
	}
	for (std::map<int, Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
		delete it->second;
		close(it->first);
	}
	for (std::map<std::string, Channel*>::iterator it = _channels.begin(); it != _channels.end(); ++it) {
		delete it->second;
	}
}

bool Server::start() {
	return setupServer();
}

void Server::run() {
	struct pollfd fds[1024];
	int nfds = 1;
	fds[0].fd = _server_fd;
	fds[0].events = POLLIN;

	while (true) {
		int ret = poll(fds, nfds, -1);
		if (ret < 0) {
			throw std::runtime_error("Polling error");
		}

		for (int i = 0; i < nfds; ++i) {
			if (fds[i].revents & POLLIN) {
				if (fds[i].fd == _server_fd) {
					acceptNewClient();
				} else {
					handleClientMessage(fds[i].fd);
				}
			}
		}

		nfds = 1;
		fds[0].fd = _server_fd;
		fds[0].events = POLLIN;
		for (std::map<int, Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
			fds[nfds].fd = it->first;
			fds[nfds].events = POLLIN;
			nfds++;
		}
	}
}

bool Server::setupServer() {
	_server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (_server_fd < 0) {
		throw std::runtime_error("Socket creation failed");
	}

	int opt = 1;
	if (setsockopt(_server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
		throw std::runtime_error("Set socket options failed");
	}

	_address.sin_family = AF_INET;
	_address.sin_addr.s_addr = INADDR_ANY;
	_address.sin_port = htons(_port);

	if (bind(_server_fd, (struct sockaddr*)&_address, sizeof(_address)) < 0) {
		throw std::runtime_error("Bind failed");
	}

	if (listen(_server_fd, 10) < 0) {
		throw std::runtime_error("Listen failed");
	}

	fcntl(_server_fd, F_SETFL, O_NONBLOCK);
	return true;
}

void Server::acceptNewClient() {
	int new_socket = accept(_server_fd, NULL, NULL);
	if (new_socket < 0) {
		return;
	}

	fcntl(new_socket, F_SETFL, O_NONBLOCK);
	_clients[new_socket] = new Client(new_socket);

	std::cout << "New client connected: " << new_socket << std::endl;
}

void Server::handleClientMessage(int client_fd) {
	char buffer[1024];
	int valread = read(client_fd, buffer, 1024);
	if (valread <= 0) {
		disconnectClient(client_fd);
		return;
	}

	buffer[valread] = '\0';
	std::string message(buffer);

	// 뒤 공백 및 개행 문자 제거
	message.erase(message.find_last_not_of(" \n\r\t") + 1);

	std::istringstream iss(message);
	std::string command;
	iss >> command;
	std::string params;
	getline(iss, params);
	params.erase(0, params.find_first_not_of(" "));	 // 앞 공백 제거

	std::map<std::string, void (Server::*)(int, const std::string&)>::iterator it = _commands.find(command);
	if (it != _commands.end()) {
		(this->*(it->second))(client_fd, params);
	} else {
		sendToClient(client_fd, "Unknown command: " + command + "\r\n");
	}
}

void Server::disconnectClient(int client_fd) {
	std::cout << "Client disconnected: " << client_fd << std::endl;
	for (std::map<std::string, Channel*>::iterator it = _channels.begin(); it != _channels.end(); ++it) {
		it->second->removeClient(_clients[client_fd]);
	}
	close(client_fd);
	delete _clients[client_fd];
	_clients.erase(client_fd);
}

void Server::sendToClient(int client_fd, const std::string& message) {
	send(client_fd, message.c_str(), message.size(), 0);
}

void Server::broadcastToChannel(const std::string& channel, const std::string& message, int except_fd) {
	if (_channels.find(channel) != _channels.end()) {
		std::vector<Client*> clients = _channels[channel]->getClients();
		for (size_t i = 0; i < clients.size(); ++i) {
			if (clients[i]->getFd() != except_fd) {
				sendToClient(clients[i]->getFd(), message);
			}
		}
	}
}
