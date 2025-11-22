#include <iostream>
#include <fstream>
#include "Server.hpp"
#include "Debug.hpp"
#include <cstdlib>

int main(int argc, char** argv)
{
	if (argc != 3)
	{
		Debug::print(ERROR, "Usage: " + std::string(argv[0]) + " port password");
		return 1;
	}
  int port = atoi(argv[1]);
  if (port < 0) //revoir les ports autorises 
  {
  	std::cout << "Incorrect port: " << port << std::endl;
      return -1;
  }
	std::string password(argv[2]);
	Server server(password, port);
	server.RunServer();
	return 0;
}
