#ifndef INVITE_COMMAND_HPP
#define INVITE_COMMAND_HPP

#include "ACommand.hpp"

class InviteCommand : public ACommand
{
	//= Methods =//
public:
	InviteCommand(std::vector<std::string> params);
	~InviteCommand() {}
	void execute(Client *executor, Server &server);
};

#endif // INVITE_COMMAND_HPP
