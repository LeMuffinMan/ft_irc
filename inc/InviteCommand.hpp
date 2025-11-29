#ifndef INVITECOMMAND_HPP
#define INVITECOMMAND_HPP

#include "ACommand.hpp"

class InviteCommand : public ACommand
{
//= Variables =//
protected:

//= Methods =//
public:
	InviteCommand(std::vector<std::string> params);
	~InviteCommand() {}
	void execute(Client* executor, Server& server);
};

#endif  // INVITECOMMAND_HPP
