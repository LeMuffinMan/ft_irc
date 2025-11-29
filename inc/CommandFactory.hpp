#ifndef COMMAND_FACTORY_HPP
#define COMMAND_FACTORY_HPP

#include "ACommand.hpp"

class CommandFactory
{
//= Variables =//
private:

//= Methods =//
public:
	CommandFactory();
	~CommandFactory();
	static command_type findType(std::string const &command_name);
	static ACommand* createCommand(const std::string& command, const std::vector<std::string> &params);
};

#endif // COMMAND_FACTORY_HPP
