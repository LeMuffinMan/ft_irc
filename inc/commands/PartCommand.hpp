#ifndef PART_COMMAND_HPP
#define PART_COMMAND_HPP

#include "ACommand.hpp"

class PartCommand : public ACommand
{
	//= Methods =//
public:
	PartCommand(std::vector<std::string> params);
	~PartCommand() {}
	void execute(Client *executor, Server &server);
};

#endif // PART_COMMAND_HPP
