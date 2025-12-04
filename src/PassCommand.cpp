#include "PassCommand.hpp"
#include "Server.hpp"

PassCommand::PassCommand(std::vector<std::string> params)
{
	_params = params;
}

void PassCommand::execute(Client* executor, Server& server)
{
	if (_params.empty())
	{
		server.reply(executor, ERR_NEEDMOREPARAMS(executor->getNickname(), "PASS"));
		return;
	}

	if (executor->isRegistered())
	{
		server.reply(executor, ERR_ALREADYREGISTRED(executor->getNickname()));
		return;
	}

	if (executor->isPasswordCorrect())
	{
		return;
	}

	if (_params[0] == server.getPassword())
	{
		executor->setPasswordCorrect();;
	} else {
		server.reply(executor, ERR_PASSWDMISMATCH(executor->getNickname()));
		server.client_kicked(executor->fd);
		// server.removeLocalUser(executor->_localClient->fd);
	}
	return;
}
