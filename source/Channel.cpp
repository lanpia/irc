#include "Channel.hpp"

Channel::Channel(const std::string& name)
	: _name(name) {
}

Channel::~Channel() {
}

// Getter
std::string Channel::getName() const { return _name; }
std::string Channel::getTopic() const { return _topic; }
std::string Channel::getMode() const { return _mode; }

// Setter
void Channel::setTopic(const std::string& topic) { _topic = topic; }
void Channel::setMode(const std::string& mode) { _mode = mode; }

// 클라이언트 관리
void Channel::addClient(Client* client) {
	if (_mode.find('i') == std::string::npos || _invitedClients.find(client->getNickname()) != _invitedClients.end()) {
		_clients.insert(client);
	}
}

void Channel::removeClient(Client* client) {
    _clients.erase(client);
}

bool Channel::isOperator(Client* client) const {
	return _operators.find(client) != _operators.end();
}

void Channel::addOperator(Client* client) {
	_operators.insert(client);
}

void Channel::removeOperator(Client* client) {
	_operators.erase(client);
}

bool Channel::kickClient(const std::string& nickname) {
	for (std::set<Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
		if ((*it)->getNickname() == nickname) {
			removeClient(*it);
			// break;
			std::cout << "Client kicked: " << nickname << std::endl;
			return true;
		}
	}
	return false;
}

void Channel::inviteClient(const std::string& nickname) {
	_invitedClients.insert(nickname);
}

bool Channel::isClientInChannel(Client* client) const {
    return _clients.find(client) != _clients.end();
}

void Channel::broadcast(const std::string& message, int except_fd) {
	for (std::set<Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
		if ((*it)->getFd() != except_fd) {
			(*it)->sendMessage(message);
		}
	}
}