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
		server.setupServer();
		server.run();
	} catch (const std::exception& e) {
		std::cerr << "Error: " << e.what() << std::endl;
		return 1;
	}

	return 0;
}
