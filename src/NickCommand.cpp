#include "NickCommand.hpp"
#include "Server.hpp"
#include <sstream>
#include "Reply.hpp"

NickCommand::NickCommand(std::vector<std::string> params)
{
	_params = params;
}

void NickCommand::execute(Client *executor, Server &server)
{

	(void)server;
	std::vector<int> vec;
	if (!executor->isPasswordCorrect())
	{
		server.reply(executor, "PASS must be first");
		server.client_kicked(executor->fd);
		return;
	}
	if (_params.empty())
	{
		server.reply(executor, ERR_NONICKNAMEGIVEN(executor->getNickname()));
		return;
	}

	std::string nick = _params[0];

	if (nick.size() > 29 || nick == "admin" || nick == "root" || nick == "operator")
	{
		server.reply(executor, ERR_ERRONEUSNICKNAME(executor->getNickname(), _params[0]));
		return;
	}

	for (size_t i = 0; i < nick.size(); ++i)
	{
		char c = nick[i];
		if (!isalnum(c) && c != '-' && c != '_')
		{
			server.reply(executor, ERR_ERRONEUSNICKNAME(executor->getNickname(), _params[0]));
			return;
		}
	}

	if (server.getClient(_params[0]))
	{
		server.reply(executor, ERR_NICKNAMEINUSE(executor->getNickname(), _params[0]));
		return;
	}

	if (!executor->getNickname().empty())
	{
		std::stringstream ss;
		ss << executor->getNickname() << " updated his nickname to " << _params[0];
		Debug::print(INFO, ss.str());
		server.reply(executor, ss.str());
	}
	executor->setNickname(_params[0]);
	return;
}

// ERR_NICKCOLLISION-> netword seulement
// ERR_UNAVAILRESOURCE	-> admin / root ...
// ERR_RESTRICTED -> pour un statut de guest
