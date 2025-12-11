#include <iostream>
#include <fstream>
#include "Server.hpp"
#include "Client.hpp"
#include "Debug.hpp"
#include <cstdlib>
#include <sstream>

int main(int argc, char **argv)
{
	if (argc != 3)
	{
		std::stringstream ss;
		ss << "Usage: " << argv[0] << " <port> <password>";
		Debug::print(ERROR, ss.str());
		return -1;
	}
	int port = atoi(argv[1]); // Needs to be replaced as atoi is not 'cpp like'
	if (port < 1024 || port > 49151)
	{
		std::stringstream ss;
		ss << "Incorrect port: " << port << ". Port must be between 1024 and 49151.";
		Debug::print(ERROR, ss.str());
		return -1;
	}
	std::string password(argv[2]);
	Server server(port, password);
	try 
	{
		if (server.RunServer() == EXIT_FAILURE)
			return -1;
	}
	catch (const std::exception &e)
	{
		std::cerr << RED << "[ERROR]" << RESET << "Impossible event to recover: " << e.what() << '\n';
	}
	return 0;
}
