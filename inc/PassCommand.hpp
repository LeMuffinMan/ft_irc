#ifndef PASSCOMMAND_HPP
#define PASSCOMMAND_HPP

#include "ACommand.hpp"

class PassCommand : public ACommand
{
//= Variables =//
protected:

//= Methods =//
public:
	PassCommand(std::vector<std::string> params);
	~PassCommand() {};
	void execute(Client* executor, NetworkState& network);
};

#endif
