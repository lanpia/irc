#include <stdlib.h>

#include <exception>
#include <iostream>

#include "Server.hpp"

int main(int argc, char* argv[]) {
	try {
		if (argc != 3) {
			throw std::runtime_error("Usage: <port> <password>");
		}

		Server server(std::atoi(argv[1]), argv[2]);
		server.setupServer();  // 생성자에서 초기화할 경우에는 예외가 발생하면 객체가 생성되지 않아서 예외 처리가 어려워짐, 따라서 setupServer()를 호출하여 초기화
		server.run();
	} catch (const std::exception& e) {
		std::cerr << "Error: " << e.what() << std::endl;
		return 1;
	}

	return 0;
}
