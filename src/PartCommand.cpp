#include "PartCommand.hpp"
#include <sstream>

PartCommand::PartCommand(std::vector<std::string> params)
{
	_params = params;
}

void PartCommand::execute(Client* executor, Server& server)
{
	if (_params.size() < 1) {
		server.reply(executor, ERR_NEEDMOREPARAMS(executor->getNickname(), "Part"));
		return;
	}
	if (!executor->isRegistered())
	{
		server.reply(executor, ERR_NOTREGISTERED(executor->getNickname()));
		return;
	}
	std::vector<std::string> channel_names;
	std::stringstream ss_1(_params[0]);
	std::string channel_name;
	char del = ',';
	while (getline(ss_1, channel_name, del))
		channel_names.push_back(channel_name);
	for (size_t i = 0; i < channel_names.size(); ++i)
	{
		Channel* channel = server.getChannel(channel_names[i]);
		if (!channel)
		{
			server.reply(executor, ERR_NOSUCHCHANNEL(executor->getNickname(), channel_names[i]));
			continue;
		}
		if (!channel->isClientInChannel(executor))
		{
			server.reply(executor, ERR_NOTONCHANNEL(executor->getNickname(), channel_names[i]));
			continue;
		}
		// Send part message to channel
		std::string part_msg = ":" + executor->getNickname() + " PART " + channel_names[i];
		server.replyChannel(channel, part_msg);
		channel->removeClient(executor);
	}
}
