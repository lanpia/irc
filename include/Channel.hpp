// 채널의 클라이언트 관리, 주제 설정, 모드 설정 등

#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <set>
#include <string>
#include <vector>
#include <cstdlib>
#include "Client.hpp"

class Channel {
   public:
	Channel(const std::string& name);
	~Channel();

	// Getter
	std::string getName() const;
	std::string getTopic() const;
	std::string getMode() const;
	unsigned int getClient() const;
	bool isInviteonly() const;
	std::string getPasswd() const;
	unsigned int getLimit() const;

	// Setter
	void setTopic(const std::string& topic);
	void setMode(const std::string& mode);
	void setInviteonly(bool isInviteonly);
	void setPasswd(const std::string& key);
	void setLimit(unsigned int limit);

	// 클라이언트 관리
	void addClient(Client* client);
	void removeClient(Client* client);
	bool isOperator(Client* client) const;
	void addOperator(Client* client);
	void removeOperator(Client* client);
	bool kickClient(const std::string& nickname);
	void inviteClient(const std::string& nickname);
	bool isClientInChannel(Client* client) const;
	void broadcast(const std::string& message, int except_fd = -1);

	void settingChannel(Client *Client, bool inviteFlag, const std::string& key, unsigned int limit, std::string topic);
	void ChangeMode(const std::string& mode);
   private:
	std::string _name;						// 채널 이름
	std::string _topic;						// 채널 주제
	std::string _mode;						// 채널 모드
	std::set<Client*> _clients;				// 채널에 있는 클라이언트 목록
	std::set<Client*> _operators;			// 채널 운영자 목록
	std::set<std::string> _invitedClients;	// 초대된 클라이언트 목록

	bool Inviteonly;
	std::string passwd;
	unsigned int limit;

};

#endif	// CHANNEL_HPP
