#include "NickCommand.hpp"
#include "Server.hpp"

NickCommand::NickCommand(std::vector<std::string> params)
{
	_params = params;
}

void NickCommand::execute(Client* executor, Server& server)
{

	(void)server;
	std::vector<int> vec;
	if (!executor->isPasswordCorrect())
	{
		vec.push_back(0); // on fait une erreur ?
		return;
	}
	if (_params.empty())
	{
		vec.push_back(431); // ERR_NONICKNAMEGIVEN
		return;
	}
	if (_params[0].empty())
	{
		vec.push_back(432); //ERR_ERRONEUSNICKNAME
		// Ca peut etre aussi des char interdits ... ou trop long 
		return; 
	}

	if (server.getClient(_params[0]))
	{
		server.reply(executor, ERR_NICKNAMEINUSE(executor->getNickname(), _params[0]));
		// vec.push_back(433); // ERR_NICKNAMEINUSE 
    	return;
	}
	// std::cout << "New nickname : " << _params[1] << std::endl;
	executor->setNickname(_params[0]);
	vec.push_back(0);
	return;
}

// ERR_NICKCOLLISION-> netword seulement
// ERR_UNAVAILRESOURCE  -> admin / root ...           
// ERR_RESTRICTED -> pour un statut de guest 
