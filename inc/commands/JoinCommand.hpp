#ifndef JOIN_COMMAND_HPP
#define JOIN_COMMAND_HPP

#include "ACommand.hpp"

class JoinCommand : public ACommand
{
	//= Methods =//
public:
	JoinCommand(std::vector<std::string> params);
	~JoinCommand() {}
	void execute(Client *executor, Server &server);
};

#endif // JOIN_COMMAND_HPP
