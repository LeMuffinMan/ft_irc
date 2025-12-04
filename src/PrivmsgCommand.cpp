#include "PrivmsgCommand.hpp"
#include "Server.hpp"

PrivmsgCommand::PrivmsgCommand(std::vector<std::string> params)
{
	_params = params;
}

// supporter l;envoie a des channels, si operator ?
void PrivmsgCommand::execute(Client *executor, Server &server)
{
	std::vector<int> vec;
	(void)server;

	if (!executor->isRegistered())
		return;
	if (_params.size() > 2)
	{
		// server.reply(executor, ERR_TOOMANYTARGETS(executor->getNickname(), _params[0], 407, abort ?))
		return;
	}
	if (_params.empty())
	{
		server.reply(executor, ERR_NORECIPIENT(executor->getNickname(), "PRIVMSG"));
		return;
	}

	if (_params.size() < 2)
	{
		server.reply(executor, ERR_NOTEXTTOSEND(executor->getNickname()));
		return;
	}
	if (_params[0][0] == '#')
	{
		Channel *target = server.getChannel(_params[0]);
		if (!target)
		{
			server.reply(executor, ERR_NOSUCHCHANNEL(executor->getNickname(), _params[0]));
			return;
		}
		if (!target->isClientInChannel(executor))
		{
			server.reply(executor, ERR_CANNOTSENDTOCHAN(executor->getNickname(), _params[0]));
			return;
		}

		// reply channel ici
		return;
	}
	else
	{

		Client *target = server.getClient(_params[0]);

		if (!target)
		{
			server.reply(executor, ERR_NOSUCHNICK(executor->getNickname(), _params[0]));
		}
		// reply au user ici
		return;
	}
}
