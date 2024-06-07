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
	std::string& getMessageBuffer();
	bool isAuthenticated() const;

	// Setter
	void setNickname(const std::string& nickname);
	void setUsername(const std::string& username);
	void setAuthenticated(bool auth);

	std::map<std::string, std::string> getBuffer(int fd);

	bool isValidNickname(const std::string& nickname) const;
	void sendMessage(const std::string& message) const;
   private:
	int _fd;						  // 클라이언트 소켓 파일 디스크립터
	bool _authenticated;			  // 클라이언트 인증 여부
	std::string _nickname;			  // 클라이언트 닉네임
	std::string _username;			  // 클라이언트 사용자 이름
	std::string _messageBuffer;		  // 클라이언트로부터 받은 메시지 버퍼
};

#endif	// CLIENT_HPP
