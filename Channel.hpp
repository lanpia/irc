#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <string>
#include <vector>
#include <set>
#include "Client.hpp"

class Channel {
public:
    Channel(const std::string& name);
    ~Channel();

    std::string getName() const;
    std::string getTopic() const;
    void setTopic(const std::string& topic);
    std::string getMode() const;
    void setMode(const std::string& mode);

    void addClient(Client* client);
    void removeClient(Client* client);
    bool isOperator(Client* client) const;
    void addOperator(Client* client);
    void removeOperator(Client* client);
    void kickClient(const std::string& nickname);
    void inviteClient(const std::string& nickname);
    std::vector<Client*> getClients() const;

private:
    std::string _name;
    std::string _topic;
    std::string _mode;
    std::vector<Client*> _clients;
    std::set<Client*> _operators;
    std::set<std::string> _invitedClients;
};

#endif // CHANNEL_HPP
