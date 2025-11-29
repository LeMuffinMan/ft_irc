#ifndef PONGCOMMAND_HPP
#define PONGCOMMAND_HPP

#include "ACommand.hpp"

class PongCommand : public ACommand
{
//= Variables =//
protected:

//= Methods =//
public:
	PongCommand(std::vector<std::string> params);
	~PongCommand() {}
	void execute(Client* executor, Server& server);
};

#endif
