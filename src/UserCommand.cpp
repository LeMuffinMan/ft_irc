#include "UserCommand.hpp"
#include "Server.hpp"

UserCommand::UserCommand(std::vector<std::string> params)
{
	_params = params;
}

void UserCommand::execute(Client* executor, Server& server)
{
	std::vector<int> vec;

	(void)server;
	if (!executor->isPasswordCorrect())
	{
		vec.push_back(0); // on repond rien, on peut aussi avoir nos propres exit code ?
		return;
	}
	if (executor->isRegistered())
	{
		vec.push_back(462); // ERR_ALREADYREGISTRED
		return;
	}

	if (_params.size() < 4) // on veut 0 et * quand meme ?
	{
		vec.push_back(461); // ERR_NEEDMOREPARAMS
		return;
	}

	vec.push_back(0);
	// std::cout << "New username : " << _params[1] << std::endl;
	executor->setUsername(_params[0]);
	executor->setRealname(_params[3]);

	return;
}

