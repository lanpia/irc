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
bool Channel::isInviteonly() const { return Inviteonly; }
std::string Channel::getPasswd() const { return passwd; }
unsigned int Channel::getLimit() const { return limit; }

// Setter
void Channel::setTopic(const std::string& topic) { _topic = topic; }
void Channel::setMode(const std::string& mode) { _mode = mode; }
void Channel::setInviteonly(bool isInviteonly) { Inviteonly = isInviteonly; }
void Channel::setPasswd(const std::string& key) { passwd = key; }
void Channel::setLimit(unsigned int limit) { limit = limit; }


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

void Channel::settingChannel(Client *client, bool inviteFlag, const std::string& key, unsigned int limit, std::string topic) {
	addOperator(client);
	setInviteonly(inviteFlag);
	setPasswd(key);
	setLimit(limit);
	setTopic(topic);
}

void Channel::ChangeMode(const std::string& mode) {
	if (mode[0] == '+') {
		for (size_t i = 1; i < mode.size(); i++) {
			switch (mode[i]) {
				case 'i':
					setInviteonly(true);
					break;
				case 'k':
					setPasswd(mode.substr(i + 1));
					break;
				case 'l':
					setLimit(std::atoi(mode.substr(i + 1).c_str()));
					break;
				case 't':
					setTopic(mode.substr(i + 1));
					break;
				default:
					break;
			}
		}
	} else if (mode[0] == '-') {
		for (size_t i = 1; i < mode.size(); i++) {
			switch (mode[i]) {
				case 'i':
					setInviteonly(false);
					break;
				case 'k':
					setPasswd("");
					break;
				case 'l':
					setLimit(0);
					break;
				case 't':
					setTopic("");
					break;
				default:
					break;
			}
		}
	}
}
