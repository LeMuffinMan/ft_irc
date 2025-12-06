#include "QuitCommand.hpp"
#include <sstream>

QuitCommand::QuitCommand(std::vector<std::string> params)
{
	_params = params;
}

void QuitCommand::execute(Client* executor, Server& server)
{
	if (_params.size() < 1) {
		server.reply(executor, ERR_NEEDMOREPARAMS(executor->getNickname(), "Quit"));
		return;
	}
	server.reply(executor, ":" + executor->getNickname() + " QUIT :" + _params[0]);
	server.removeClient(executor);
}
