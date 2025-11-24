#include "InviteCommand.hpp"

InviteCommand::InviteCommand(std::vector<std::string> params)
{
	_params = params;
}

void InviteCommand::execute(Client* executor, NetworkState& network)
{
	if (_params.size() < 2) {
		// ERR_NEEDMOREPARAMS
		return;
	}
	Client* target = network.getClient(_params[0]);
    Channel* channel = network.getChannel(_params[1]);
    if (!target || !channel) {
		// ERR_NOSUCHNICK
		Debug::print(DEBUG, "Invalid params");
        // executor.sendError("Invalid params");
        return;
    }
	if (!channel->isClientInChannel(executor))
	{
		// ERR_NOTONCHANNEL
		Debug::print(DEBUG, "Client is not in channel");
		// executor.sendError("Client is not in channel");
		return;
	}
	if (channel->isClientInChannel(target))
	{
		// ERR_USERONCHANNEL
		Debug::print(DEBUG, "Target user is already in channel");
		// executor.sendError("Target user is already in channel");
		return;
	}
	if (channel->getModes().is_invite_only)
	{
		if (channel->isClientOPChannel(executor))
		{
			if (!target)
			{
				// RPL_AWAY
				return;
			}
			// RPL_INVITING
			channel->addClient(target);
			return;
		}
		else
		{
			// ERR_CHANOPRIVSNEEDED
			Debug::print(DEBUG, "Client is not channel operator");
			// executor.sendError("Client is not channel operator");
			return;
		}
	}
	else // Not invite only
	{
		if (!target)
		{
			// RPL_AWAY
			return;
		}
		// RPL_INVITING
		channel->addClient(target);
		return;
	}
}
