#include "PassCommand.hpp"
#include "LocalUser.hpp"
#include "Server.hpp"

PassCommand::PassCommand(std::vector<std::string> params)
{
	_params = params;
}

void PassCommand::execute(Client* executor, Server& server)
{
	std::vector<int> vec;

	if (_params.empty() || _params[0].empty())
		vec.push_back(461); // ERR_NEEDMOREPARAMS
	if (executor->_registered)
		vec.push_back(462); // ERR_ALREADYREGISTRED

	if (!vec.empty())
		return;

  if (executor->_password_correct)
	{
		vec.push_back(0);
		return;
	}

	if (_params[0] == server.getPassword())
	{
		// std::cout << "Correct password" << std::endl;
	  executor->_password_correct = true;
	  vec.push_back(0);
	} else {
		vec.push_back(464); //ERR_PASSWDMISMATCH
	}
	return;
}
