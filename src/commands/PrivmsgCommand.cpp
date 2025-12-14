#include "PrivmsgCommand.hpp"
#include <sstream>
#include "Server.hpp"

PrivmsgCommand::PrivmsgCommand(std::vector<std::string> params)
{
	_params = params;
}

/**
 * @brief Execute the Privmsg command.
 * Users communicate using Privmsg, directly to user sharing the same server,
 * or channel, or on an entire channel.
 * Ex : Privmsg target_nickname :msg
 * 	  : Privmsg #channel :msg
 * 
 * @param[in,out] executor client executing the command
 * @param[in,out] server server instance
 */
void PrivmsgCommand::execute(Client *executor, Server &server)
{
	std::vector<int> vec;
	(void)server;

	if (!executor->isRegistered())
	{
		server.reply(executor, ERR_NOTREGISTERED(executor->getNickname()));
		return;
	}
	if (_params.size() > 2)
	{
		server.reply(executor, ERR_TOOMANYTARGETS(executor->getNickname(), _params[0], "407"));
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
		std::stringstream ss;
		ss << ":" << executor->getHost() << " " << "PRIVMSG " << target->getName() << " :" << _params[1];
		server.replyChannel(target, ss.str());
		return;
	}
	else
	{
		Client *target = server.getClient(_params[0]);

		if (!target)
		{
			server.reply(executor, ERR_NOSUCHNICK(executor->getNickname(), _params[0]));
			return;
		}
		std::stringstream ss;
		ss << ":" << executor->getHost() << " " << "PRIVMSG "
			<< target->getNickname()  << " :" 
			<< _params[1];
		server.reply(target, ss.str());
		return;
	}
}
