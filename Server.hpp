#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <netinet/in.h>
#include "Client.hpp"
#include "Channel.hpp"

class Server {
public:
    Server(int port, const std::string& password);
    ~Server();

    bool start();
    void run();

private:
    int _port;
    std::string _password;
    int _server_fd;
    struct sockaddr_in _address;
    std::map<int, Client*> _clients;
    std::map<std::string, Channel*> _channels;

    bool setupServer();
    void acceptNewClient();
    void handleClientMessage(int client_fd);
    void disconnectClient(int client_fd);
    void sendToClient(int client_fd, const std::string& message);
    void broadcastToChannel(const std::string& channel, const std::string& message, int except_fd = -1);
};

#endif // SERVER_HPP
