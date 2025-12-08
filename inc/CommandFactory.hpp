#ifndef COMMAND_FACTORY_HPP
#define COMMAND_FACTORY_HPP

#include "ACommand.hpp"

class CommandFactory
{
//= Methods =//
private:
	CommandFactory();
	~CommandFactory();
	static std::string get_command(const std::string &line);
	static std::vector<std::string> get_params(const std::string &line);
	static command_type findType(std::string const &command_name);
	static ACommand *createCommand(const std::string &command, const std::vector<std::string> &params);

public:
	static ACommand *findAndCreateCommand(const std::string &line);
};

#endif // COMMAND_FACTORY_HPP
