#include "UserCommand.hpp"
#include "Server.hpp"

UserCommand::UserCommand(std::vector<std::string> params)
{
	_params = params;
}

void UserCommand::execute(Client* executor, NetworkState& network)
{
	std::vector<int> vec;

	(void)network;
	if (!executor->_password_correct)
	{
		vec.push_back(0); // on repond rien, on peut aussi avoir nos propres exit code ?
		return;
	}
	if (executor->_registered)
	{
		vec.push_back(462); // ERR_ALREADYREGISTRED
		return;
	}

	if (_params.size() < 5) // on veut 0 et * quand meme ?
	{
		vec.push_back(461); // ERR_NEEDMOREPARAMS
		return;
	}

	vec.push_back(0);
	// std::cout << "New username : " << _params[1] << std::endl;
  executor->_username = _params[1];
  executor->_realname = _params[4];

	return;
}

