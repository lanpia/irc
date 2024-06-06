#include "Channel.hpp"

#include <algorithm>

Channel::Channel(const std::string& name)
	: _name(name) {
}

Channel::~Channel() {
}

// Getter
std::string Channel::getName() const { return _name; }
std::string Channel::getTopic() const { return _topic; }
std::string Channel::getMode() const { return _mode; }
std::vector<Client*> Channel::getClients() const { return _clients; }

// Setter
void Channel::setTopic(const std::string& topic) { _topic = topic; }
void Channel::setMode(const std::string& mode) { _mode = mode; }

// 클라이언트 관리
void Channel::addClient(Client* client) {
	if (_mode.find('i') == std::string::npos || _invitedClients.find(client->getNickname()) != _invitedClients.end()) {
		_clients.push_back(client);
	}
}

void Channel::removeClient(Client* client) {
	_clients.erase(std::remove(_clients.begin(), _clients.end(), client), _clients.end());
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
	for (std::vector<Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
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
