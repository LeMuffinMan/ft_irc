#include <iostream>
#include <fstream>
#include "Server.hpp"
#include "Client.hpp"
#include "LocalUser.hpp"
#include "Debug.hpp"
#include "ACommand.hpp"
#include "CommandFactory.hpp"

int main(int argc, char** argv)
{
	std::string command_type;
	std::vector<std::string> params;
	command_type = "INVITE";
	params.push_back("user1");
	params.push_back("user2");
	ACommand *command = CommandFactory::createCommand(command_type, params);
	if (command)
	{
		command->execute();
		delete command;
	}
	else
	{
		Debug::print(ERROR, "Failed to create command.");
	}
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
