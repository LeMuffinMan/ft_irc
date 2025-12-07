#include "PongCommand.hpp"
#include "Server.hpp"
#include <sstream>
#include <cstdlib>

PongCommand::PongCommand(std::vector<std::string> params)
{
	_params = params;
}

void PongCommand::execute(Client *executor, Server &server)
{
	(void)server;

	if (executor->last_ping != std::atol(_params[0].c_str()))
	{
		server.reply(executor, "Invalid pong");
		server.removeClient(executor->fd);
		return;
	}
	std::time_t now = std::time(NULL);
	executor->timeout = now + 5;
	executor->last_ping = now;

	std::stringstream ss;
	ss << "[PONG: " << _params[0]
	   << "] from client " << executor->fd
	   << " received";

	Debug::print(DEBUG, ss.str());
}
