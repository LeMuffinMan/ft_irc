#include <iostream>
#include <fstream>
#include "Server.hpp"
#include "Debug.hpp"

int main(int argc, char** argv)
{
	if (argc != 3)
	{
		Debug::print(ERROR, "Usage: " + std::string(argv[0]) + " port password");
		return 1;
	}
	std::string port(argv[1]);
	std::string password(argv[2]);
	Server server(port, password);
	server.RunServer();
	return 0;
}
