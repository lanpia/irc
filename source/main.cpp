/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nahyulee <nahyulee@student.42seoul.kr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/14 01:38:18 by nahyulee          #+#    #+#             */
/*   Updated: 2024/06/17 23:59:07 by nahyulee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

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
		server.ServerRun();
	} catch (const std::exception& e) {
		std::cerr << "Error: " << e.what() << std::endl;
		return 1;
	}
	return 0;
}
