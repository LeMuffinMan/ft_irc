#include "UserCommand.hpp"
#include "Server.hpp"

UserCommand::UserCommand(std::vector<std::string> params)
{
	_params = params;
}

/**
 * @brief Execute the User command.
 * User can claim a username, to identify itself on channels and server, 
 * Ex : User username hostname servername :realname
 *
 */



void UserCommand::execute(Client *executor, Server &server)
{
	if (!executor->isPasswordCorrect())
	{
		server.reply(executor, "PASS must be first");
		server.client_kicked(executor->_fd);
		return;
	}

	if (_params.size() < 4)
	{
		server.reply(executor, ERR_NEEDMOREPARAMS(executor->getNickname(), "USER"));
		return;
	}

	if (executor->isRegistered())
	{
		server.reply(executor, ERR_ALREADYREGISTRED(executor->getNickname()));
		return;
	}
	executor->setUsername(_params[0]);
	executor->setRealname(_params[3]);
	return;
}
