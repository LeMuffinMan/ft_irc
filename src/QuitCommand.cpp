#include "QuitCommand.hpp"
#include <sstream>

QuitCommand::QuitCommand(std::vector<std::string> params)
{
	_params = params;
}

/**
 * @brief Execute the QUIT command.
 * Ex: QUIT :Quit message
 * Disconnect the client from the server with a quit message.
 *
 * @param[in,out] executor client executing the command
 * @param[in,out] server server instance
 */
void QuitCommand::execute(Client *executor, Server &server)
{
	if (_params.size() < 1)
	{
		server.reply(executor, ERR_NEEDMOREPARAMS(executor->getNickname(), "Quit"));
		return;
	}
	server.reply(executor, ":" + executor->getNickname() + " QUIT :" + _params[0]);
	server.removeClientFromAllChannels(executor); // FIXME: could be placed in server.removeClient
	server.removeClient(executor);
}
