/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nahyulee <nahyulee@student.42seoul.kr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/08 19:58:12 by nahyulee          #+#    #+#             */
/*   Updated: 2024/06/25 13:46:02 by nahyulee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP

#include <netinet/in.h>
#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <arpa/inet.h>
#include <fcntl.h>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>
#include <sstream>
#include "Channel.hpp"
#include "Client.hpp"

class Server {
private:
	Server();
	Server(const Server& copy);
	Server& operator=(const Server& copy);
	
	int svrFd;
	std::string password;
	struct sockaddr_in _address;
	std::map<int, Client*> clients;
	std::map<std::string, Channel*> channels;
	void acceptNewClient();
	std::map<std::string, void (Server::*)(int, const std::string&, const std::string&)> _commands;
public:
	~Server();
	Server(int port, const std::string& password);
	class ServerException : public std::runtime_error {
    public:
        explicit ServerException(const std::string& message) : std::runtime_error(message) {}
		virtual ~ServerException() throw() {}
    };
	void ServerRun();
	void disconnectClient(int client_fd);
	void handleClientMessage(int client_fd);
	int findClientFd(const std::string& target);

	void handlePass(int client_fd, const std::string& target, const std::string& message);
	void handleNick(int client_fd, const std::string& target, const std::string& message);
	void handleUser(int client_fd, const std::string& target, const std::string& message);
	void handleJoin(int client_fd, const std::string& target, const std::string& message);
	void handlePart(int client_fd, const std::string& target, const std::string& message);
	void handlePrivmsg(int client_fd, const std::string& target, const std::string& message);
	void handleKick(int client_fd, const std::string& target, const std::string& message);
	void handleInvite(int client_fd, const std::string& target, const std::string& message);
	void handleTopic(int client_fd, const std::string& target, const std::string& message);
	void handleMode(int client_fd, const std::string& target, const std::string& message);
	void handleQuit(int client_fd, const std::string& target, const std::string& message);
};

#endif