#include "InviteCommand.hpp"

InviteCommand::InviteCommand(std::vector<std::string> params)
{
	_params = params;
}

void InviteCommand::execute(Client* executor, Server& server)
{
	if (_params.size() < 2) {
		server.reply(executor, ERR_NEEDMOREPARAMS(executor->getNickname(), "INVITE"));
		return;
	}
	Client* target = server.getClient(_params[0]);
	Channel* channel = server.getChannel(_params[1]);
	if (!target || !channel) {
		server.reply(executor, ERR_NOSUCHNICK(executor->getNickname(), _params[1]));
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
			channel->addClient(target);
			server.reply(executor, RPL_INVITING(executor->getNickname(), _params[1], _params[0]));
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
		channel->addClient(target);
		server.reply(executor, RPL_INVITING(executor->getNickname(), _params[1], _params[0]));
		return;
	}
}
