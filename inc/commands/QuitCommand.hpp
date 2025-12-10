#ifndef QUIT_COMMAND_HPP
#define QUIT_COMMAND_HPP

#include "ACommand.hpp"

class QuitCommand : public ACommand
{
	//= Methods =//
public:
	QuitCommand(std::vector<std::string> params);
	~QuitCommand() {}
	void execute(Client *executor, Server &server);
};

#endif // QUIT_COMMAND_HPP
