#ifndef KICKCOMMAND_HPP
#define KICKCOMMAND_HPP

#include "ACommand.hpp"

class KickCommand : public ACommand
{
//= Variables =//
protected:

//= Methods =//
public:
	KickCommand(std::vector<std::string> params);
	~KickCommand() {};
	void execute(Client* executor, NetworkState& network, Server& server);
};

#endif
