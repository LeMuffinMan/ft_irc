#include "InviteCommand.hpp"


InviteCommand::InviteCommand(std::vector<std::string> params)
{
	_params = params;
}

void InviteCommand::execute()
{
	this->print_params();
	// Implementation of the invite command execution
}
