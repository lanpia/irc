// 채널의 클라이언트 관리, 주제 설정, 모드 설정 등

#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <set>
#include <string>
#include <vector>

#include "Client.hpp"

class Channel {
   public:
	Channel(const std::string& name);
	~Channel();

	// Getter
	std::string getName() const;
	std::string getTopic() const;
	std::string getMode() const;
	std::vector<Client*> getClients() const;

	// Setter
	void setTopic(const std::string& topic);
	void setMode(const std::string& mode);

	// 클라이언트 관리
	void addClient(Client* client);
	void removeClient(Client* client);
	bool isOperator(Client* client) const;
	void addOperator(Client* client);
	void removeOperator(Client* client);
	bool kickClient(const std::string& nickname);
	void inviteClient(const std::string& nickname);

   private:
	std::string _name;						// 채널 이름
	std::string _topic;						// 채널 주제
	std::string _mode;						// 채널 모드
	std::vector<Client*> _clients;			// 채널에 있는 클라이언트 목록
	std::set<Client*> _operators;			// 채널 운영자 목록
	std::set<std::string> _invitedClients;	// 초대된 클라이언트 목록
};

#endif	// CHANNEL_HPP
