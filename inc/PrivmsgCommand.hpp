#ifndef PRIVMSGCOMMAND_HPP
#define PRIVMSGCOMMAND_HPP

#include "ACommand.hpp"

class PrivmsgCommand : public ACommand
{
//= Variables =//
protected:

//= Methods =//
public:
	PrivmsgCommand(std::vector<std::string> params);
	~PrivmsgCommand() {};
	void execute(Client* executor, NetworkState& network);
};

#endif
