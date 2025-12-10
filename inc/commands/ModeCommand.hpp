#ifndef MODE_COMMAND_HPP
#define MODE_COMMAND_HPP

#include "ACommand.hpp"

class ModeCommand : public ACommand
{
//= Variables =//
protected:

//= Methods =//
public:
	ModeCommand(std::vector<std::string> params);
	~ModeCommand() {}
	void execute(Client* executor, Server& server);

private:
	void executeUserMode(Client* executor, Server& server);
	void executeChannelMode(Client* executor, Server& server);
};

#endif // MODE_COMMAND_HPP
