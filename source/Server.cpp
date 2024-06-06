#include "Server.hpp"

#include <arpa/inet.h>
#include <fcntl.h>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <sstream>

Server::Server(int port, const std::string& password)
	: _port(port), _password(password), _server_fd(-1) {

	// SeverCommands.cpp에 있는 명령어 처리 함수들을 _commands에 매핑
	// C++ 98에서는 중괄호 초기화가 불가능하기에, 인덱스 연산자를 사용하여 초기화
	_commands["NICK"] = &Server::handleNick;
	_commands["USER"] = &Server::handleUser;
	_commands["JOIN"] = &Server::handleJoin;
	_commands["PART"] = &Server::handlePart;
	_commands["PRIVMSG"] = &Server::handlePrivmsg;
	_commands["KICK"] = &Server::handleKick;
	_commands["INVITE"] = &Server::handleInvite;
	_commands["TOPIC"] = &Server::handleTopic;
	_commands["MODE"] = &Server::handleMode;
	_commands["QUIT"] = &Server::handleQuit;
	// responseCode
	// :<서버 이름> <응답 코드> <대상 사용자> :<메시지 내용>
	_responseCode["001"] = ":Welcome to the Internet Relay Network ";
	_responseCode["002"] = ":Your host is ";
	_responseCode["003"] = ":This server was created ";
	_responseCode["251"] = ":There are ";
	_responseCode["252"] = ":operator(s) online";
	_responseCode["253"] = ":unknown connection(s)";
	_responseCode["254"] = ":channels formed";
	_responseCode["255"] = ":I have ";
	_responseCode["265"] = ":Current Local Users: ";
	_responseCode["266"] = ":Current Global Users: ";
	_responseCode["332"] = " :No topic is set";
	_responseCode["333"] = " ";
	_responseCode["401"] = " :No such nick/channel";
	_responseCode["402"] = " :No such server";
	_responseCode["403"] = " :No such channel";
	_responseCode["404"] = " :Cannot send to channel";
	_responseCode["405"] = " :You have joined too many channels";
	_responseCode["406"] = " :There was no such nickname";
	_responseCode["407"] = " :Too many recipients";
	_responseCode["408"] = " :No text to send";
	_responseCode["409"] = " :No origin specified";
	_responseCode["411"] = " :No recipient given";
	_responseCode["412"] = " :No text to send";
	_responseCode["421"] = " :Unknown command";
	_responseCode["422"] = " :MOTD File is missing";
	_responseCode["431"] = " :No nickname given";
	_responseCode["432"] = " :Erroneous nickname";
	_responseCode["433"] = " :Nickname is already in use";
	_responseCode["436"] = " :Nickname collision KILL";
	_responseCode["441"] = " :They aren't on that channel";
	_responseCode["442"] = " :You're not on that channel";
	_responseCode["443"] = " :is already on channel";
	_responseCode["444"] = " :User not logged in";
	_responseCode["445"] = " :SUMMON has been disabled";
	_responseCode["446"] = " :USERS has been disabled";
	_responseCode["451"] = " :You have not registered";
	_responseCode["461"] = " :Not enough parameters";
	_responseCode["462"] = " :You may not reregister";
	_responseCode["463"] = " :Your host isn't among the privileged";
	_responseCode["464"] = " :Password incorrect";
	_responseCode["465"] = " :You are banned from this server";
	_responseCode["466"] = " :You will be banned from this server";
	_responseCode["467"] = " :Channel key already set";
	_responseCode["471"] = " :Cannot join channel (+l)";
	_responseCode["472"] = " :is unknown mode char to me";
	_responseCode["481"] = " :Permission Denied- You're not an IRC operator";
	_responseCode["482"] = " :You're not channel operator";
	_responseCode["483"] = " :You cant kill a server";
	_responseCode["491"] = " :No O-lines for your host";
	_responseCode["501"] = " :Unknown MODE flag";
	_responseCode["502"] = " :Cannot change mode for other users";
	_responseCode["503"] = " :Your IRC operator privileges are too low";
	_responseCode["504"] = " :Cannot kill server";
	_responseCode["511"] = " :Channel doesn't exist";
	_responseCode["512"] = " :You're not an IRC operator";
	_responseCode["513"] = " :is already registered";
	_responseCode["514"] = " :Your host is already registered";
	
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

/* 사전 지식
 *
 * 소켓이란?
 *	소켓은 네트워크 통신을 위한 엔드포인트를 의미합니다. 소켓은 파일 디스크립터와 유사하게 사용할 수 있습니다.
 *
 * 네트워크 통신 방식
 * 1. TCP (Transmission Control Protocol) <--- 본 과제에서 사용됨
 * 	- 연결 지향적이며 신뢰성 있는 통신 방식입니다.
 * 	- 데이터의 전송 순서가 보장됩니다.
 * 	- 데이터의 손실이 발생하면 재전송합니다.
 * 	- 3-way handshake를 통해 연결을 설정하고, 4-way handshake를 통해 연결을 해제합니다.
 * 2. UDP (User Datagram Protocol)
 * 	- 비연결 지향적이며 비신뢰성 있는 통신 방식입니다.
 * 	- 데이터의 전송 순서가 보장되지 않습니다.
 * 	- 데이터의 손실이 발생하면 손실된 데이터를 재전송하지 않습니다.
 * 	- 연결 설정 및 해제 과정이 없습니다.
 * 	- 실시간 스트리밍, DNS 등에서 사용됩니다.
 *
 * 네트워크 바이트 순서
 * 	- 네트워크 바이트 순서는 빅 엔디안 방식을 사용합니다.
 * 	- 빅 엔디안 방식은 가장 상위 비트부터 저장하는 방식입니다.
 * 	- 예를 들어, 0x12345678을 저장하면 0x12 0x34 0x56 0x78 순서로 저장합니다.
 * 	  또한 0x87654321을 저장하면 0x87 0x65 0x43 0x21 순서로 저장합니다. 즉, 빅엔디안 방식은 순서대로 저장하는 방식입니다.
 */
void Server::setupServer() {
	/* int socket(int domain, int type, int protocol)
	 *	설명: 소켓을 생성합니다.
	 *
	 *	domain: 소켓의 주소 체계를 지정합니다. AF_INET은 IPv4 주소 체계를 의미합니다.
	 *	type: 소켓의 타입을 지정합니다. SOCK_STREAM은 TCP 소켓을 의미합니다.
	 *	protocol: 소켓의 프로토콜을 지정합니다. 0은 기본 프로토콜을 사용함을 의미합니다.
	 *
	 *	반환값: 소켓의 파일 디스크립터를 반환합니다. 실패 시 -1을 반환합니다.
	 */
	_server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (_server_fd < 0) {
		throw std::runtime_error("Socket creation failed");
	}

	/* int setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen)
	 *	설명: 소켓의 옵션을 설정합니다.
	 *
	 *	sockfd: 옵션을 설정할 소켓의 파일 디스크립터를 지정합니다.
	 *	level: 옵션의 프로토콜 레벨을 지정합니다. SOL_SOCKET은 일반 소켓 옵션을 의미합니다.
	 *	optname: 설정할 옵션의 이름을 지정합니다. SO_REUSEADDR은 포트 재사용을 의미합니다.
	 *	optval: 옵션의 값을 지정합니다. 1로 설정하면 포트 재사용이 가능합니다.
	 *	optlen: 옵션의 값의 크기를 지정합니다.
	 *
	 *	반환값: 성공 시 0을 반환합니다. 실패 시 -1을 반환합니다.
	 */
	int opt = 1;
	if (setsockopt(_server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
		throw std::runtime_error("Set socket options failed");
	}

	/* struct sockaddr_in _address에 대한 설명
	 * 주소 정보를 저장하는 구조체입니다.
	 *
	 * struct sockaddr_in {
	 *	short sin_family;			// 주소 체계, AF_INET은 IPv4 주소 체계
	 *	unsigned short sin_port;	// 포트 번호, htons() 함수로 네트워크 바이트 순서로 변환
	 *	struct in_addr sin_addr;	// IP 주소, INADDR_ANY는 모든 IP 주소를 의미
	 *	char sin_zero[8];			// 사용되지 않고 구조체 크기를 맞추기 위한 더미 필드
	 * };
	 */
	_address.sin_family = AF_INET;
	_address.sin_addr.s_addr = INADDR_ANY;
	_address.sin_port = htons(_port);

	/* int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
	 *	설명: 소켓에 주소를 할당합니다.
	 *
	 * 	sockfd: 주소를 할당할 소켓의 파일 디스크립터를 지정합니다.
	 * 	addr: 할당할 주소를 지정합니다.
	 * 	addrlen: 주소의 크기를 지정합니다.
	 *
	 * 	반환값: 성공 시 0을 반환합니다. 실패 시 -1을 반환합니다.
	 */
	if (bind(_server_fd, (struct sockaddr*)&_address, sizeof(_address)) < 0) {
		throw std::runtime_error("Bind failed");
	}

	/* int listen(int sockfd, int backlog)
	 *	설명: 소켓을 수신 대기 상태로 설정합니다.
	 *
	 * 	sockfd: 수신 대기 상태로 설정할 소켓의 파일 디스크립터를 지정합니다.
	 * 	backlog: 대기 큐의 최대 길이를 지정합니다.
	 *
	 * 	반환값: 성공 시 0을 반환합니다. 실패 시 -1을 반환합니다.
	 */
	if (listen(_server_fd, 10) < 0) {
		throw std::runtime_error("Listen failed");
	}

	/* fcntl(int fd, int cmd, ...)
	 *	설명: 파일 디스크립터의 특성을 변경합니다.
	 *
	 * 	fd: 특성을 변경할 파일 디스크립터를 지정합니다.
	 * 	cmd: 변경할 특성을 지정합니다. F_SETFL은 파일 디스크립터의 플래그를 설정하는 명령어입니다.
	 * 	...: 변경할 플래그를 지정합니다. O_NONBLOCK은 블록되지 않는 소켓을 의미합니다.
	 *
	 * 	반환값: 성공 시 0을 반환합니다. 실패 시 -1을 반환합니다.
	 */
	if (fcntl(_server_fd, F_SETFL, O_NONBLOCK) < 0) {
		throw std::runtime_error("Set non-blocking failed");
	}
}

void Server::run() {
	/* struct pollfd fds[512]에 대한 설명
	 * poll() 함수에 사용할 파일 디스크립터 목록입니다.
	 *
	 * struct pollfd {
	 *	int fd;		// 파일 디스크립터
	 *	short events;	// 이벤트 종류, POLLIN은 읽기 가능 이벤트를 의미합니다.
	 *	short revents;	// 발생한 이벤트 종류
	 * };
	 */
	struct pollfd fds[255];
	int nfds = 1;
	fds[0].fd = _server_fd;
	fds[0].events = POLLIN;

	while (true) {
		/* int poll(struct pollfd fds[], nfds_t nfds, int timeout)
		 *	설명: 파일 디스크립터의 이벤트를 대기합니다.
		 *
		 * 	fds: 이벤트를 대기할 파일 디스크립터 목록을 지정합니다.
		 * 	nfds: 파일 디스크립터 목록의 크기를 지정합니다.
		 * 	timeout: 대기 시간을 지정합니다. -1은 무한 대기를 의미합니다.
		 *
		 * 	반환값: 발생한 이벤트의 개수를 반환합니다. 실패 시 -1을 반환합니다.
		*/
		int ret = poll(fds, nfds, -1);
		if (ret < 0) {
			throw std::runtime_error("Polling error");
		}

		for (int i = 0; i < nfds; ++i) {
			if (!(fds[i].revents & POLLIN)) {
				continue;
			}
			if (fds[i].fd == _server_fd) {
				acceptNewClient();
			} else {
				handleClientMessage(fds[i].fd);
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

void Server::acceptNewClient() {
	int new_socket = accept(_server_fd, NULL, NULL);
	if (new_socket < 0) {
		return;
	}

	fcntl(new_socket, F_SETFL, O_NONBLOCK);
	_clients[new_socket] = new Client(new_socket);

	std::cout << "New client connected: " << new_socket << std::endl;
}

void Server::handleClientMessage(int client_fd) {
	char buffer[513];
	int valread = read(client_fd, buffer, 512);
	if (valread <= 0) {
		disconnectClient(client_fd);
		return;
	}
	
	buffer[valread] = '\0';
	std::string &messageBuffer = _clients[client_fd]->getMessageBuffer();
	messageBuffer.append(buffer, valread);

	size_t pos;
	if ((pos = messageBuffer.find("\r\n")) == std::string::npos) {	// 메시지의 끝을 \r\n으로 인식
		return;
	}

	if (messageBuffer.size() > 512) {
		sendToClient(client_fd, "Message too long\n");
		messageBuffer.clear();
		return;
	}

	std::string message = messageBuffer.substr(0, pos);
	messageBuffer.clear();

    if (message[0] == ' ') {
        sendToClient(client_fd, "ERROR :Invalid command\r\n");
		return;
    }

	message.erase(message.find_last_not_of(" \n\r\t") + 1);

	std::istringstream iss(message);
	std::string command;
	iss >> command;
	std::string params;
	if (getline(iss, params)) {
		params.erase(0, 1);
	}

	std::map<std::string, void (Server::*)(int, const std::string&)>::iterator it = _commands.find(command);
	if (it != _commands.end()) {
		(this->*(it->second))(client_fd, params);
	} else {
		sendToClient(client_fd, "Unknown command: " + command + "\r\n");
	}
}

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

bool Server::isSettingUser(int client_fd) {
	if (_clients[client_fd]->getNickname().empty() || _clients[client_fd]->getUsername().empty()) {
		sendToClient(client_fd, "ERROR :You need to set your nickname and username first\r\n");
		return false;
	} else {
		return true;
	}
}