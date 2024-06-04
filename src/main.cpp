#include <stdlib.h>

#include <exception>
#include <iostream>

#include "Server.hpp"

int main(int argc, char* argv[]) {
	try {
		if (argc != 3) {
			throw std::runtime_error("Usage: <port> <password>");
		}

		// Server Argument: Port number, Password
		Server server(std::atoi(argv[1]), argv[2]);
		if (!server.start()) {
			throw std::runtime_error("Failed to start the server.");
		}

		server.run();
	} catch (const std::exception& e) {
		std::cerr << "Error: " << e.what() << std::endl;
		return 1;
	}

	return 0;
}
