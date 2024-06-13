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
		_clients[client_fd]->sendMessage("Nickname already in use\r\n");
		return;
	}

	if (_clients[client_fd]->isValidNickname(params) == false) {
		_clients[client_fd]->sendMessage("Invalid nickname\r\n");
		return;
	}

	_clients[client_fd]->setNickname(params);
	_clients[client_fd]->sendMessage("NICK " + _clients[client_fd]->getNickname() + "\r\n");
}

void Server::handleUser(int client_fd, const std::string& params) {
	if (isAlreadyUsed(_clients, &Client::getUsername, params)) {
		_clients[client_fd]->sendMessage("Username already in use\r\n");
		return;
	}
	_clients[client_fd]->setUsername(params);
	_clients[client_fd]->sendMessage("USER " + _clients[client_fd]->getUsername() + "\r\n");
}

void Server::handleJoin(int client_fd, const std::string& params) {
	// struct exception_cases {
	// 	void *condition;
	// 	bool expected;
	// 	std::string exceptMsg;
	// } {
	// 	_clients[client_fd]->getNickname().empty(), true, "You have to setting Nickname",
	// 	_clients[client_fd]->getUsername().empty(), true, "You have to setting Username",
	// 	_clients[client_fd]->getNickname().empty() || _clients[client_fd]->getUsername().empty(),
	// 	_clients[client_fd]->getNickname().empty() && _clients[client_fd]->getUsername().empty()
	// };
	
	// for (; ; ) {
	// 	if (exception_cases.condition == exception_cases.expected) {
	// 		_clients[client_fd]->sendMessage(exception_cases.exceptMsg + "\r\n");
	// 		break;
	// 	}
	// }

	std::string channelName = params;
	std::string passwd;
	size_t pos = channelName.find(" ");
	if (pos != std::string::npos) {
		passwd = channelName.substr(pos + 1);
		channelName = channelName.substr(0, pos);
	}
	if (_clients[client_fd]->getNickname().empty() || _clients[client_fd]->getUsername().empty()) {
		_clients[client_fd]->sendMessage("You have to setting Username, Nickname\r\n");
		return;
	}
	if (_channels.find(channelName) == _channels.end()) {
		_channels[channelName] = new Channel(channelName);
		_channels[channelName]->broadcast("Channel created: " + channelName + "\r\n");
		// _channels[channelName]->addOperator(_clients[client_fd]);
		_channels[channelName]->settingChannel(_clients[client_fd], false, "", 0, "");
		_clients[client_fd]->sendMessage("Now youer Channel operator\r\n");
		_channels[channelName]->broadcast("you have to Setting TOPIC\r\n");
	}
	// if (_channels[channelName]->isInviteonly() == true) {
	// 	_clients[client_fd]->sendMessage("JOIN fail\r\n");
	// 	return ;
	// }
	// if(_channels[channelName]->getLimit() != 0 && _channels[channelName]->getLimit() <= _channels[channelName]->getClients().size()) {
	// 	_clients[client_fd]->sendMessage("Channel is full\r\n");
	// 	return ;
	// }
	if (_channels[channelName]->checkMode(params) != "Success") {
		// exception(std::string )
		_clients[client_fd]->sendMessage(_channels[channelName]->checkMode(params) + "\r\n");
		return;
	}
	_channels[channelName]->addClient(_clients[client_fd]);
	_clients[client_fd]->sendMessage("JOIN " + channelName + "\r\n");
	_channels[channelName]->broadcast(_clients[client_fd]->getNickname() + " has joined the channel\r\n", client_fd);
	if (_channels[channelName]->getTopic().empty() == false)
		_channels[channelName]->broadcast("TOPIC " + channelName + " :" + _channels[channelName]->getTopic() + "\r\n");
}

void Server::handlePart(int client_fd, const std::string& params) {
	std::string channelName = params;
	if (_channels.find(channelName) != _channels.end()) {
		_channels[channelName]->removeClient(_clients[client_fd]);
		_clients[client_fd]->sendMessage("PART " + channelName + "\r\n");
		_channels[channelName]->broadcast(_clients[client_fd]->getNickname() + " has left the channel\r\n", client_fd);
	}
}

void Server::handlePrivmsg(int client_fd, const std::string& params) {
	size_t pos = params.find(" ");
	std::string target = params.substr(0, pos);
	std::string msg = params.substr(pos + 1);
	if (target[0] == '#') {
		// if (!isClientInChannel(*_clients[client_fd], target)) {
		if (!_channels[target]->isClientInChannel(_clients[client_fd])) {
			_clients[client_fd]->sendMessage("You are not in the channel\r\n");
			return;
		}
		// 채널 메시지
		_channels[target]->broadcast("PRIVMSG " + target + " :" + _clients[client_fd]->getNickname() + ": " + msg + "\r\n", client_fd);
	} else {
		// 개인 메시지
		for (std::map<int, Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
			if (it->second->getNickname() == target) {
				_clients[client_fd]->sendMessage("PRIVMSG " + target + " :" + _clients[client_fd]->getNickname() + ": " + msg + "\r\n");
			}
		}
	}
}

void Server::handleKick(int client_fd, const std::string& params) {
	size_t pos = params.find(" ");
	std::string channelName = params.substr(0, pos);
	std::string target = params.substr(pos + 1);
	// if (!isClientInChannel(*_clients[client_fd], target)) {
	if (!_channels[channelName]->isClientInChannel(_clients[client_fd])) {
		_clients[client_fd]->sendMessage("You are not in the channel\r\n");
		return;
	}
	if (_channels.find(channelName) != _channels.end() && _channels[channelName]->isOperator(_clients[client_fd])) {
		// _channels[channelName]->kickClient(target);
		_channels[channelName]->removeClient(_clients[client_fd]);
		_clients[client_fd]->sendMessage("KICK " + channelName + " " + target + "\r\n");
		_channels[channelName]->broadcast(target + " has been kicked from the channel\r\n");
	}
}

void Server::handleInvite(int client_fd, const std::string& params) {
	size_t pos = params.find(" ");
	std::string target = params.substr(0, pos);
	std::string channelName = params.substr(pos + 1);
	if (!_channels[channelName]->isClientInChannel(_clients[client_fd])) {
		_clients[client_fd]->sendMessage("You are not in the channel\r\n");
		return;
	}
	if (_channels.find(channelName) != _channels.end() && _channels[channelName]->isOperator(_clients[client_fd])) {
		_channels[channelName]->inviteClient(target);
		_clients[client_fd]->sendMessage("INVITE " + target + " " + channelName + "\r\n");
	}
}

void Server::handleTopic(int client_fd, const std::string& params) {
	size_t pos = params.find(" ");
	std::string channelName = params.substr(0, pos);
	std::string topic = params.substr(pos + 1);
	if (!_channels[channelName]->isClientInChannel(_clients[client_fd])) {
		_clients[client_fd]->sendMessage("You are not in the channel\r\n");
		return;
	}
	if (_channels.find(channelName) != _channels.end() && _channels[channelName]->isOperator(_clients[client_fd])) {
		_channels[channelName]->setTopic(topic);
		_clients[client_fd]->sendMessage("TOPIC " + channelName + " :" + topic + "\r\n");
		_channels[channelName]->broadcast("TOPIC " + channelName + " :" + topic + "\r\n");
	}
}

void Server::handleMode(int client_fd, const std::string& params) {
	size_t pos = params.find(" ");
	std::string channelName = params.substr(0, pos);
	std::string mode = params.substr(pos + 1);
	std::cout << pos << channelName << mode << std::endl;
	if (!_channels[channelName]->isClientInChannel(_clients[client_fd])) {
		_clients[client_fd]->sendMessage("You are not in the channel\r\n");
		return;
	}
	// if (_channels[channelName]->checkMode(params) != "Success")
	if (_channels.find(channelName) != _channels.end() && _channels[channelName]->isOperator(_clients[client_fd])) {
		_channels[channelName]->setMode(mode);
		_channels[channelName]->ChangeMode(mode);
		_clients[client_fd]->sendMessage("MODE " + channelName + " " + mode + "\r\n");
		_channels[channelName]->broadcast("MODE " + channelName + " " + mode + "\r\n");
	}
}

void Server::handleQuit(int client_fd, const std::string& params) {
	(void)params;
	disconnectClient(client_fd);
}
