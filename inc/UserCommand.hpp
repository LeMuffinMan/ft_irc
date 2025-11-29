#ifndef USERCOMMAND_HPP
#define USERCOMMAND_HPP

#include "ACommand.hpp"

class UserCommand : public ACommand
{
//= Variables =//
protected:

//= Methods =//
public:
	UserCommand(std::vector<std::string> params);
	~UserCommand() {}
	void execute(Client* executor, Server& server);
};

#endif
