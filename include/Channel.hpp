/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nahyulee <nahyulee@student.42seoul.kr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/08 18:38:16 by nahyulee          #+#    #+#             */
/*   Updated: 2024/06/17 23:46:01 by nahyulee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <set>
#include <map>
#include <string>
#include <vector>
#include <cstdlib>
#include <sys/types.h> 
#include <sys/socket.h>
#include <stdexcept>

class Client;

class Channel {

private:
    Channel();
    Channel(const Channel& copy);
    Channel& operator=(const Channel& copy);

    std::string ChatInfo[5];
    std::set<Client*> clients;

public:
    enum e_info { chatname, topic, limits, passwd, inviteOnly };
    Channel(const std::string& name);
    ~Channel();
    bool isValidChatname(const std::string& name) const;
    std::string is(int idx) const;
    void set(int idx, const std::string opt, const std::string& str);
	std::set<Client*> getClients();
	bool emptyClient() const;
	bool LastOperator() const;
    class ChannelException : public std::runtime_error {
    public:
        explicit ChannelException(const std::string& message) : std::runtime_error(message) {}
        virtual ~ChannelException() throw() {}
    };
    void ClientInOut(std::string inout, Client* client) throw(Channel::ChannelException);
    void broadcast(const std::string& message, int except_fd);
    void broadcast(const std::string& message);
};

#endif
