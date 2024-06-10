/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nahyulee <nahyulee@student.42seoul.kr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/08 19:50:19 by nahyulee          #+#    #+#             */
/*   Updated: 2024/06/08 23:22:27 by nahyulee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <set>
#include <string>
#include <map>
#include <iostream>
#include <sstream>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h> // ssize_t recv(int sockfd, void *buf, size_t len, int flags);

// template <typename T1, typename T2>
// struct Pair {
//     T1 first;
//     T2 second;
//     Pair(const T1& f, const T2& s) : first(f), second(s) {}
// };

template <typename T1, typename T2, typename T3>
struct Triple {
    T1 first;
    T2 second;
    T3 third;
    Triple(const T1& f, const T2& s, const T3& t) : first(f), second(s), third(t) {}
};

class Client {
private:
	Client();
	Client(const Client& copy);
	Client& operator=(const Client& copy);
	
	enum e_info{nickname, username, Operator};
	int fd;
	std::string ClientInfo[3];

public:
	Client(int fd);
	~Client();
	class ClientException : public std::exception {
	public:
		virtual ~ClientException() throw() { const char *what(); }
	};
	void set(enum e_info idx, const std::string opt, const std::string& str);
	std::string is(enum e_info idx) const;
	
	bool isValidNickname(const std::string& nickname) const;
	Triple<std::string, std::string, std::string> parseMessage();
};

#endif
