#include "TimeCommand.hpp"
#include <sstream>

TimeCommand::TimeCommand(std::vector<std::string> params)
{
	_params = params;
}

void TimeCommand::execute(Client* executor, Server& server)
{
	if (_params.size() >= 1) {
		server.reply(executor, "Time command does not take any parameters");
		return;
	}
	std::stringstream ss;
	ss << format_date() << " -- " << format_time() << " +00:00";
	server.reply(executor, RPL_TIME(executor->getNickname(), executor->getNickname(), ss.str()));
}
