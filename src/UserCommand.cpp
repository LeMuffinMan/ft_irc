#include "UserCommand.hpp"
#include "Server.hpp"

UserCommand::UserCommand(std::vector<std::string> params)
{
	_params = params;
}

void UserCommand::execute(Client* executor, Server& server)
{
	(void)server;
	if (!executor->isPasswordCorrect())
	{
		//mdp not first
		return;
	}

	if (_params.size() < 4) // on veut 0 et * quand meme ?
	{
		server.reply(executor, ERR_NEEDMOREPARAMS(executor->getNickname(), "USER"));
		return;
	}


	if (executor->isRegistered())
	{
		server.reply(executor, ERR_ALREADYREGISTRED(executor->getNickname()));
		return;
	}
	//on accepte deux username identique ?

	// std::cout << "New username : " << _params[1] << std::endl;
	executor->setUsername(_params[0]);
	executor->setRealname(_params[3]);

	return;
}

