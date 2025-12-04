#include <iostream>
#include <fstream>
#include "Server.hpp"
#include "Client.hpp"
#include "Debug.hpp"
#include <cstdlib>

#include <cstdlib>
#include <string>

int main(int argc, char** argv)
{
	if (argc != 3)
	{
		std::cerr << "Usage: ./ircserv <port> <password>" << std::endl;
		return -1;
	}
	int port = atoi(argv[1]); // Needs to be replaced as atoi is not 'cpp like'
	if (port < 1024 || port > 49151)
	{
		std::cerr << "Incorrect port: " << port << std::endl;
		return -1;
	}
	std::string password(argv[2]);
	Server server(port, password);
	server.RunServer();
	return 0;
}
