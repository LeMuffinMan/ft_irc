#include "InviteCommand.hpp"

InviteCommand::InviteCommand(std::vector<std::string> params)
{
	_params = params;
}

void InviteCommand::execute(Client* executor, NetworkState& network)
{
	Client* target = network.getClient(_params[0]);
    Channel* channel = network.getChannel(_params[1]);
    if (!target || !channel) {
		Debug::print(DEBUG, "Invalid params");
        // executor.sendError("Invalid params");
        return;
    }
	if (!channel->isClientInChannel(executor))
	{
		Debug::print(DEBUG, "Client is not in channel");
		// executor.sendError("Client is not in channel");
		return;
	}
	if (channel->getModes().is_invite_only && channel->isClientOPChannel(executor))
	{
		channel->addClient(target);
	}
	else if (channel->getModes().is_invite_only && !channel->isClientOPChannel(executor))
	{
		Debug::print(DEBUG, "Client is not channel operator");
		// executor.sendError("Client is not channel operator");
		return;
	}
	else
	{
		channel->addClient(target);
	}
}
