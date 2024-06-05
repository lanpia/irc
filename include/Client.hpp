//  클라이언트의 닉네임 설정, 유저네임 설정, 채널 참여/탈퇴 등

#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <set>
#include <string>
#include <map>

#include <iostream>
#include <sstream>

#include <unistd.h>
#include <fcntl.h>

class Client {
   public:
	Client(int fd);
	~Client();

	// Getter
	int getFd() const;
	std::string getNickname() const;
	std::string getUsername() const;
	bool isInChannel(const std::string& channel) const;

	// Setter
	void setNickname(const std::string& nickname);
	void setUsername(const std::string& username);

	// 채널 관련
	void joinChannel(const std::string& channel);
	void leaveChannel(const std::string& channel);
	std::map<std::string, std::string> getBuffer(int fd);

   private:
	int _fd;						  // 클라이언트 소켓 파일 디스크립터
	std::string _nickname;			  // 클라이언트 닉네임
	std::string _username;			  // 클라이언트 사용자 이름
	std::set<std::string> _channels;  // 클라이언트가 참여한 채널 목록
};

#endif	// CLIENT_HPP
