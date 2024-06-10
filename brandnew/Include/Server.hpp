/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nahyulee <nahyulee@student.42seoul.kr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/08 19:58:12 by nahyulee          #+#    #+#             */
/*   Updated: 2024/06/10 13:22:07 by nahyulee         ###   ########.fr       */
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
	int socket;
	int port;
	std::string password;
	std::vector<Client*> clients;
	std::map<std::string, Channel*> channels;
	

public:
	~Server();
	Server(int port, const std::string& password);

	class ServerException : public std::exception {
	public:
		virtual ~ServerException() throw() { const char *what(); }
	};

	void sendMessage(const std::string& message) const;
	void broadcast(const std::string& message, int except_fd = -1);
};

#endif
