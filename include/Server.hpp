// 서버 소켓을 설정하고, 클라이언트 연결을 수락하며, 클라이언트 메시지를 처리

#ifndef SERVER_HPP
#define SERVER_HPP

#include <netinet/in.h>

#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "Channel.hpp"
#include "Client.hpp"

class Server {
   public:
	Server(int port, const std::string& password);
	~Server();

	void run();
	void setupServer();

   private:
	int _port;																	 // 서버가 청취할 포트 번호
	std::string _password;														 // 클라이언트가 연결할 때 사용할 비밀번호
	int _server_fd;																 // 서버 소켓 파일 디스크립터
	struct sockaddr_in _address;												 // 서버 주소 정보
	std::map<int, Client*> _clients;											 // 클라이언트 목록
	std::map<std::string, Channel*> _channels;									 // 채널 목록
	std::map<std::string, void (Server::*)(int, const std::string&)> _commands;	 // 커맨드 매핑

	// responseCode
	std::map<std::string, std::string> _responseCode;
	void Server::printServerInfo(std::string codenum, Client* _client);

	void acceptNewClient();
	void handleClientMessage(int client_fd);
	void disconnectClient(int client_fd);

	// 명령어 처리 함수
	void handleNick(int client_fd, const std::string& params);
	void handleUser(int client_fd, const std::string& params);
	void handleJoin(int client_fd, const std::string& params);
	void handlePart(int client_fd, const std::string& params);
	void handlePrivmsg(int client_fd, const std::string& params);
	void handleKick(int client_fd, const std::string& params);
	void handleInvite(int client_fd, const std::string& params);
	void handleTopic(int client_fd, const std::string& params);
	void handleMode(int client_fd, const std::string& params);
	void handleQuit(int client_fd, const std::string& params);
};

#endif	// SERVER_HPP
