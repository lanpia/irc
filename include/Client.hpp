/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nahyulee <nahyulee@student.42seoul.kr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/08 19:50:19 by nahyulee          #+#    #+#             */
/*   Updated: 2024/06/26 22:53:59 by nahyulee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
# define CLIENT_HPP 

# include <set>
# include <string>
# include <map>
# include <vector>
# include <iostream>
# include <sstream>
# include <cstdlib>
# include <algorithm>
# include <unistd.h>
# include <fcntl.h>
# include <sys/socket.h>
# include <sys/types.h>

#define ANSI_RED     "\x1b[31m"
#define ANSI_GREEN   "\x1b[32m"
#define ANSI_YELLOW  "\x1b[33m"
#define ANSI_BLUE    "\x1b[34m"
#define ANSI_MAGENTA "\x1b[35m"
#define ANSI_CYAN    "\x1b[36m"
#define ANSI_RESET   "\x1b[0m"

template <typename T1, typename T2, typename T3>
struct Triple {
    T1 first;
    T2 second;
    T3 third;
	Triple() {}
    Triple(const T1& f, const T2& s, const T3& t) : first(f), second(s), third(t) {}
};

class Client {
private:
	Client();
	Client(const Client& copy);
	Client& operator=(const Client& copy);
	
	int fd;
	std::string ClientInfo[4];

public:
	enum e_info{Nickname, Username, Chatname, Operator};
	Client(int fd);
	~Client();
	class ClientException : public std::runtime_error {
    public:
        explicit ClientException(const std::string& message) : std::runtime_error(message) {}
		virtual ~ClientException() throw() {}
    };
	void set(int idx, const std::string opt, const std::string& str);
	std::string is(int idx) const;
	int getFd() const;
	bool isValidNickname(const std::string& nickname) const;
	bool checkDefaultInfo(int level) const;
	void sendMessage(const std::string& message) const;
	void sendMessage(const std::string& message, std::string color) const;
	Triple<std::string, std::string, std::string> parseMessage();
	std::vector<std::string> MODEcount(const std::string& message);
	Triple<int, std::string, std::string> MODEparse(const char mode, std::vector<std::string>* token, const char sign);
};

#endif
