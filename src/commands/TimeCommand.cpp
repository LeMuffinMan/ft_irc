#include "TimeCommand.hpp"
#include <sstream>

TimeCommand::TimeCommand(std::vector<std::string> params)
{
	_params = params;
}

/**
 * @brief Execute the TIME command.
 * Ex: TIME
 * Returns the current server time.
 *
 * @param[in,out] executor client executing the command
 * @param[in,out] server server instance
 */
void TimeCommand::execute(Client *executor, Server &server)
{
	if (_params.size() >= 1)
	{
		server.reply(executor, "Time command does not take any parameters");
		return;
	}
	std::stringstream ss;
	ss << format_date() << " -- " << format_time() << " +00:00";
	server.reply(executor, RPL_TIME(executor->getNickname(), executor->getNickname(), ss.str()));
}
