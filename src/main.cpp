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
	(void)argc;
	(void)argv;

	// Initialize NetworkState with a client and channel components
	std::vector<Client*> clients;
	NetworkState network;
	network.addClient("user1", new Client());
	Client* user1 = network.getClient("user1");
	network.addChannel("channel");
	Channel* channel = network.getChannel("channel");
	channel->addClient(user1);
	if (channel)
		std::cout << "Channel found !" << std::endl;
	else
		Debug::print(DEBUG, "Failed to find channel.");

	// Create and execute an InviteCommand by user1
	std::string command_type;
	std::vector<std::string> params;
	command_type = "INVITE";
	params.push_back("user1");
	params.push_back("channel");
	ACommand *command = CommandFactory::createCommand(command_type, params);
	if (command)
	{
		command->execute(user1, network);
		delete command;
	}
	else
	{
		Debug::print(DEBUG, "Failed to create command.");
	}
	return 0;
}
