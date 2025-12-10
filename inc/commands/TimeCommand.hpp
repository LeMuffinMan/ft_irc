#ifndef TIME_COMMAND_HPP
#define TIME_COMMAND_HPP

#include "ACommand.hpp"

class TimeCommand : public ACommand
{
	//= Methods =//
public:
	TimeCommand(std::vector<std::string> params);
	~TimeCommand() {}
	void execute(Client *executor, Server &server);
};

#endif // TIME_COMMAND_HPP
