#include <sstream>

#include "Server.hpp"

static bool isAlreadyUsed(const std::map<int, Client*>& clients, std::string (Client::*func)() const, const std::string& params) {
    for (std::map<int, Client*>::const_iterator it = clients.begin(); it != clients.end(); ++it) {
        if ((it->second->*func)() == params) {
            return true;
        }
    }
    return false;
}

void Server::handleNick(int client_fd, const std::string& params) {
	if (isAlreadyUsed(_clients, &Client::getNickname, params)) {
		sendToClient(client_fd, "Nickname already in use\r\n");
		return;
	}
	_clients[client_fd]->setNickname(params);
	sendToClient(client_fd, "NICK " + _clients[client_fd]->getNickname() + "\r\n");
}

void Server::handleUser(int client_fd, const std::string& params) {
	if (isAlreadyUsed(_clients, &Client::getUsername, params)) {
		sendToClient(client_fd, "Username already in use\r\n");
		return;
	}
	_clients[client_fd]->setUsername(params);
	sendToClient(client_fd, "USER " + _clients[client_fd]->getUsername() + "\r\n");
}

void Server::handleJoin(int client_fd, const std::string& params) {
	std::string channelName = params;
	if (_clients[client_fd]->getNickname().empty() || _clients[client_fd]->getUsername().empty()) {
		sendToClient(client_fd, "You have to setting Username, Nickname\r\n");
		return;
	}
	if (_channels.find(channelName) == _channels.end()) {
		_channels[channelName] = new Channel(channelName);
		broadcastToChannel(channelName, "Channel created: " + channelName + "\r\n");
		broadcastToChannel(channelName, "you have to Setting TOPIC\r\n");
	}
	_channels[channelName]->addClient(_clients[client_fd]);
	_clients[client_fd]->joinChannel(channelName);
	sendToClient(client_fd, "JOIN " + channelName + "\r\n");
	broadcastToChannel(channelName, _clients[client_fd]->getNickname() + " has joined the channel\r\n", client_fd);
	if (_channels[channelName]->getTopic().empty() == false)
		broadcastToChannel(channelName, "TOPIC " + channelName + " :" + _channels[channelName]->getTopic() + "\r\n");
}

void Server::handlePart(int client_fd, const std::string& params) {
	std::string channelName = params;
	if (_channels.find(channelName) != _channels.end()) {
		_channels[channelName]->removeClient(_clients[client_fd]);
		_clients[client_fd]->leaveChannel(channelName);
		sendToClient(client_fd, "PART " + channelName + "\r\n");
		broadcastToChannel(channelName, _clients[client_fd]->getNickname() + " has left the channel\r\n", client_fd);
	}
}

static bool isUserInChannel(Client &_clients, const std::string& channelName) {
	if (!_clients.isInChannel(channelName))
		return false;
	return true;
}

void Server::handlePrivmsg(int client_fd, const std::string& params) {
	size_t pos = params.find(" ");
	std::string target = params.substr(0, pos);
	std::string msg = params.substr(pos + 1);
	if (target[0] == '#') {
		// if (target == _channels.find(target)->first) {
		// 	sendToClient(client_fd, "Cannot send message to default channel\r\n");
		// 	return;
		// }
		if (!isUserInChannel(*_clients[client_fd], target)) {
			sendToClient(client_fd, "You are not in the channel\r\n");
			return;
		}
		// 채널 메시지
		broadcastToChannel(target, "PRIVMSG " + target + " :" + _clients[client_fd]->getNickname() + ": " + msg + "\r\n", client_fd);
	} else {
		// 개인 메시지
		for (std::map<int, Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
			if (it->second->getNickname() == target) {
				sendToClient(it->first, "PRIVMSG " + target + " :" + _clients[client_fd]->getNickname() + ": " + msg + "\r\n");
			}
		}
	}
}

void Server::handleKick(int client_fd, const std::string& params) {
	size_t pos = params.find(" ");
	std::string channelName = params.substr(0, pos);
	std::string target = params.substr(pos + 1);
	if (!isUserInChannel(*_clients[client_fd], target)) {
		sendToClient(client_fd, "You are not in the channel\r\n");
		return;
	}
	if (_channels.find(channelName) != _channels.end() && _channels[channelName]->isOperator(_clients[client_fd])) {
		_channels[channelName]->kickClient(target);
		sendToClient(client_fd, "KICK " + channelName + " " + target + "\r\n");
		broadcastToChannel(channelName, target + " has been kicked from the channel\r\n");
	}
}

void Server::handleInvite(int client_fd, const std::string& params) {
	size_t pos = params.find(" ");
	std::string target = params.substr(0, pos);
	std::string channelName = params.substr(pos + 1);
	if (!isUserInChannel(*_clients[client_fd], channelName)) {
		sendToClient(client_fd, "You are not in the channel\r\n");
		return;
	}
	if (_channels.find(channelName) != _channels.end() && _channels[channelName]->isOperator(_clients[client_fd])) {
		_channels[channelName]->inviteClient(target);
		sendToClient(client_fd, "INVITE " + target + " " + channelName + "\r\n");
	}
}

void Server::handleTopic(int client_fd, const std::string& params) {
	size_t pos = params.find(" ");
	std::string channelName = params.substr(0, pos);
	std::string topic = params.substr(pos + 1);
	if (!isUserInChannel(*_clients[client_fd], channelName)) {
		sendToClient(client_fd, "You are not in the channel\r\n");
		return;
	}
	if (_channels.find(channelName) != _channels.end() && _channels[channelName]->isOperator(_clients[client_fd])) {
		_channels[channelName]->setTopic(topic);
		sendToClient(client_fd, "TOPIC " + channelName + " :" + topic + "\r\n");
		broadcastToChannel(channelName, "TOPIC " + channelName + " :" + topic + "\r\n");
	}
}

void Server::handleMode(int client_fd, const std::string& params) {
	size_t pos = params.find(" ");
	std::string channelName = params.substr(0, pos);
	std::string mode = params.substr(pos + 1);
	if (!isUserInChannel(*_clients[client_fd], channelName)) {
		sendToClient(client_fd, "You are not in the channel\r\n");
		return;
	}
	if (_channels.find(channelName) != _channels.end() && _channels[channelName]->isOperator(_clients[client_fd])) {
		_channels[channelName]->setMode(mode);
		sendToClient(client_fd, "MODE " + channelName + " " + mode + "\r\n");
		broadcastToChannel(channelName, "MODE " + channelName + " " + mode + "\r\n");
	}
}

void Server::handleQuit(int client_fd, const std::string& params) {
	(void)params;
	disconnectClient(client_fd);
}
