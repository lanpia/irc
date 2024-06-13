/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nahyulee <nahyulee@student.42seoul.kr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/08 18:38:16 by nahyulee          #+#    #+#             */
/*   Updated: 2024/06/14 00:52:52 by nahyulee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <set>
#include <string>
#include <vector>
#include <cstdlib>

class Client;

class Channel {

private:
	Channel();
	Channel(const Channel& copy);
	Channel& operator=(const Channel& copy);

	enum e_info{chatname, topic, limits, passwd, inviteOnly};
	std::string ChatInfo[5];
	
	std::set<Client*> _clients;

public:
	Channel(const std::string& name);
	~Channel();

	std::string is(enum e_info idx) const;
	void set(enum e_info idx, const std::string opt, const std::string& str);
	
	class ChannelException : public std::exception {
	public:
		virtual ~ChannelException() throw() { const char *what(); }
	};
	
	std::string ClientInOut(std::string inout, Client* client) throw(Channel::ChannelException);
	

};

#endif
