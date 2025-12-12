#include "InviteCommand.hpp"

InviteCommand::InviteCommand(std::vector<std::string> params)
{
	_params = params;
}

/**
 * @brief Execute the INVITE command.
 * Invite a user to a channel.
 * Ex: INVITE user #channel
 * The user receives an invitation to join the specified channel.
 *
 * @param[in,out] executor client executing the command
 * @param[in,out] server server instance
 */
void InviteCommand::execute(Client *executor, Server &server)
{
	if (!executor->isRegistered())
	{
		server.reply(executor, ERR_NOTREGISTERED(executor->getNickname()));
		return;
	}
	if (_params.size() < 2)
	{
		server.reply(executor, ERR_NEEDMOREPARAMS(executor->getNickname(), "INVITE"));
		return;
	}
	if (!executor->isRegistered())
	{
		server.reply(executor, ERR_NOTREGISTERED(executor->getNickname()));
		return;
	}
	Client *target = server.getClient(_params[0]);
	Channel *channel = server.getChannel(_params[1]);
	if (!target)
	{
		server.reply(executor, ERR_NOSUCHNICK(executor->getNickname(), _params[0]));
		return;
	}
	if (!channel)
	{
		server.reply(executor, ERR_NOSUCHCHANNEL(executor->getNickname(), _params[1]));
		return;
	}
	if (!channel->isClientInChannel(executor))
	{
		server.reply(executor, ERR_NOTONCHANNEL(executor->getNickname(), _params[1]));
		return;
	}
	if (channel->isClientInChannel(target))
	{
		server.reply(executor, ERR_USERONCHANNEL(executor->getNickname(), target->getNickname(), _params[1]));
		return;
	}
	if (channel->getModes().is_invite_only)
	{
		if (channel->isClientOPChannel(executor))
		{
			if (!target)
			{
				// RPL_AWAY : hors scope : commande AWAY
				return;
			}
			channel->addClientToAllowList(target);
			server.reply(executor, RPL_INVITING(executor->getNickname(), _params[1], _params[0]));
			server.reply(target, RPL_INVITEACCEPTED(target->getHost(), _params[1], _params[0]));
			return;
		}
		else
		{
			server.reply(executor, ERR_CHANOPRIVSNEEDED(executor->getNickname(), _params[1]));
			return;
		}
	}
	else // Not invite only
	{
		if (!target)
		{
			// RPL_AWAY : same
			return;
		}
		channel->addClientToAllowList(target);
		server.reply(executor, RPL_INVITING(executor->getNickname(), _params[1], _params[0]));
		server.reply(target, RPL_INVITEACCEPTED(target->getHost(), _params[1], _params[0]));
		return;
	}
}
