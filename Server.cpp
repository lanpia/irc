#include "Server.hpp"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <cstring>

Server::Server(int port, const std::string& password)
    : _port(port), _password(password), _server_fd(-1) {
}

Server::~Server() {
    if (_server_fd != -1) {
        close(_server_fd);
    }
    for (std::map<int, Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
        delete it->second;
        close(it->first);
    }
    for (std::map<std::string, Channel*>::iterator it = _channels.begin(); it != _channels.end(); ++it) {
        delete it->second;
    }
}

bool Server::start() {
    return setupServer();
}

void Server::run() {
    struct pollfd fds[1024];
    int nfds = 1;
    fds[0].fd = _server_fd;
    fds[0].events = POLLIN;

    while (true) {
        int ret = poll(fds, nfds, -1);
        if (ret < 0) {
            perror("poll");
            break;
        }

        for (int i = 0; i < nfds; ++i) {
            if (fds[i].revents & POLLIN) {
                if (fds[i].fd == _server_fd) {
                    acceptNewClient();
                } else {
                    handleClientMessage(fds[i].fd);
                }
            }
        }

        nfds = 1;
        fds[0].fd = _server_fd;
        fds[0].events = POLLIN;
        for (std::map<int, Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
            fds[nfds].fd = it->first;
            fds[nfds].events = POLLIN;
            nfds++;
        }
    }
}

bool Server::setupServer() {
    _server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (_server_fd < 0) {
        perror("socket");
        return false;
    }

    int opt = 1;
    if (setsockopt(_server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        return false;
    }

    _address.sin_family = AF_INET;
    _address.sin_addr.s_addr = INADDR_ANY;
    _address.sin_port = htons(_port);

    if (bind(_server_fd, (struct sockaddr*)&_address, sizeof(_address)) < 0) {
        perror("bind");
        return false;
    }

    if (listen(_server_fd, 10) < 0) {
        perror("listen");
        return false;
    }

    fcntl(_server_fd, F_SETFL, O_NONBLOCK);
    return true;
}

void Server::acceptNewClient() {
    int new_socket = accept(_server_fd, NULL, NULL);
    if (new_socket < 0) {
        perror("accept");
        return;
    }

    fcntl(new_socket, F_SETFL, O_NONBLOCK);
    _clients[new_socket] = new Client(new_socket);

    std::cout << "New client connected: " << new_socket << std::endl;
}
// _cmd로 사사용용할할수수있있게 정리하기 
// NICK USER 설정후 나머지 사용할 수 있게 하기
void Server::handleClientMessage(int client_fd) {
    char buffer[1024];
    int valread = read(client_fd, buffer, 1024);
    if (valread <= 0) {
        disconnectClient(client_fd);
        return;
    }

    buffer[valread] = '\0';
    std::string message(buffer);

    std::cout << "Message from client " << client_fd << ": " << message << std::endl;
    // 메시지 처리 로직 추가
    std::cout << "Make a account" << std::endl;
    std::cout << "enter the 'NICK [nickname]' plz" << std::endl;
    _clients[client_fd]->setNickname(message.substr(5));
    std::cout << "enter the 'USER [username]' plz" << std::endl;
    _clients[client_fd]->setUsername(message.substr(5));
    std::cout << "enter the 'JOIN [channel]' plz(If Channel is not exist we make the Channel)" << std::endl;
    std::string channelName = message.substr(5);
    if (_channels[channelName] == NULL) {
        _channels[channelName] = new Channel(channelName);
    }
    if (message.find("NICK") == 0) {
        // NICK 명령 처리
        _clients[client_fd]->setNickname(message.substr(5));
        sendToClient(client_fd, "NICK " + _clients[client_fd]->getNickname() + "\r\n");
    } else if (message.find("USER") == 0) {
        // USER 명령 처리
        _clients[client_fd]->setUsername(message.substr(5));
        sendToClient(client_fd, "USER " + _clients[client_fd]->getUsername() + "\r\n");
    } else if (message.find("JOIN") == 0) {
        // JOIN 명령 처리
        std::string channelName = message.substr(5);
        if (_channels.find(channelName) == _channels.end()) {
            _channels[channelName] = new Channel(channelName);
        }
        _channels[channelName]->addClient(_clients[client_fd]);
        _clients[client_fd]->joinChannel(channelName);
        sendToClient(client_fd, "JOIN " + channelName + "\r\n");
        broadcastToChannel(channelName, _clients[client_fd]->getNickname() + " has joined the channel\r\n", client_fd);
    } else if (message.find("PRIVMSG") == 0) {
        // PRIVMSG 명령 처리
        size_t pos = message.find(" ", 8);
        std::string target = message.substr(8, pos - 8);
        std::string msg = message.substr(pos + 1);
        if (target[0] == '#') {
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
    } else if (message.find("KICK") == 0) {
        // KICK 명령 처리
        size_t pos = message.find(" ", 5);
        std::string channelName = message.substr(5, pos - 5);
        std::string target = message.substr(pos + 1);
        if (_channels.find(channelName) != _channels.end() && _channels[channelName]->isOperator(_clients[client_fd])) {
            _channels[channelName]->kickClient(target);
            sendToClient(client_fd, "KICK " + channelName + " " + target + "\r\n");
            broadcastToChannel(channelName, target + " has been kicked from the channel\r\n");
        }
    } else if (message.find("INVITE") == 0) {
        // INVITE 명령 처리
        size_t pos = message.find(" ", 7);
        std::string target = message.substr(7, pos - 7);
        std::string channelName = message.substr(pos + 1);
        if (_channels.find(channelName) != _channels.end() && _channels[channelName]->isOperator(_clients[client_fd])) {
            _channels[channelName]->inviteClient(target);
            sendToClient(client_fd, "INVITE " + target + " " + channelName + "\r\n");
        }
    } else if (message.find("TOPIC") == 0) {
        // TOPIC 명령 처리
        size_t pos = message.find(" ", 6);
        std::string channelName = message.substr(6, pos - 6);
        std::string topic = message.substr(pos + 1);
        if (_channels.find(channelName) != _channels.end() && _channels[channelName]->isOperator(_clients[client_fd])) {
            _channels[channelName]->setTopic(topic);
            sendToClient(client_fd, "TOPIC " + channelName + " :" + topic + "\r\n");
            broadcastToChannel(channelName, "TOPIC " + channelName + " :" + topic + "\r\n");
        }
    } else if (message.find("MODE") == 0) {
        // MODE 명령 처리
        size_t pos = message.find(" ", 5);
        std::string channelName = message.substr(5, pos - 5);
        std::string mode = message.substr(pos + 1);
        if (_channels.find(channelName) != _channels.end() && _channels[channelName]->isOperator(_clients[client_fd])) {
            _channels[channelName]->setMode(mode);
            sendToClient(client_fd, "MODE " + channelName + " " + mode + "\r\n");
            broadcastToChannel(channelName, "MODE " + channelName + " " + mode + "\r\n");
        }
    } else if (message.find("QUIT") == 0) {
        disconnectClient(client_fd);
    }
    else if (msg.("HELP")) {
        print cmd all;
    }
}

기존 | 변경
막막 | 이름 -> aa -> aa ->

void Server::disconnectClient(int client_fd) {
    std::cout << "Client disconnected: " << client_fd << std::endl;
    for (std::map<std::string, Channel*>::iterator it = _channels.begin(); it != _channels.end(); ++it) {
        it->second->removeClient(_clients[client_fd]);
    }
    close(client_fd);
    delete _clients[client_fd];
    _clients.erase(client_fd);
}

void Server::sendToClient(int client_fd, const std::string& message) {
    send(client_fd, message.c_str(), message.size(), 0);
}

void Server::broadcastToChannel(const std::string& channel, const std::string& message, int except_fd) {
    if (_channels.find(channel) != _channels.end()) {
        std::vector<Client*> clients = _channels[channel]->getClients();
        for (size_t i = 0; i < clients.size(); ++i) {
            if (clients[i]->getFd() != except_fd) {
                sendToClient(clients[i]->getFd(), message);
            }
        }
    }
}
