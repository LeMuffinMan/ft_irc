#include <iostream>
#include <fstream>
#include "Server.hpp"
#include "Client.hpp"
#include "LocalUser.hpp"
#include "Debug.hpp"
#include <cstdlib>

int main(int argc, char** argv)
{
  if (argc != 3)
  {
    std::cout << "Usage: ./ircserv <port> <password>" << std::endl;
    return -1;
  }
  int port = atoi(argv[1]);
  if (port < 0) //revoir les ports autorises 
  {
  	std::cout << "Incorrect port: " << port << std::endl;
      return -1;
  }
	std::string password(argv[2]);
	Server server(port, password);
	server.RunServer();
	return 0;
}
