#include "PassCommand.hpp"

PassCommand::PassCommand(std::vector<std::string> params)
{
	_params = params;
}

void PassCommand::execute(Client* executor, NetworkState& network)
{
  //Comment traiter PASS password extra params ?
	if (_params.size() < 1) {
		// ERR_NEEDMOREPARAMS
		return;
	}

  if (executor.password_correct)
    return; // si il a deja renseigne un mot de passe correct

	Client* password = network.getClient(_params[0]);
	if (password != server.getPassword())
	{
	  executor.password_correct = true;
	}
	return;
}
