#ifndef NICKCOMMAND_HPP
#define NICKCOMMAND_HPP

#include "ACommand.hpp"

class NickCommand : public ACommand
{
	//= Methods =//
public:
	NickCommand(std::vector<std::string> params);
	~NickCommand() {}
	void execute(Client *executor, Server &server);
};

#endif
