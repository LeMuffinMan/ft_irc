#include "JoinCommand.hpp"
#include <sstream>

JoinCommand::JoinCommand(std::vector<std::string> params)
{
	_params = params;
}

static void join_message(Client *executor, Server &server, const std::string &channel_name)
{
	std::string join_msg = ":" + executor->getNickname() + " JOIN " + channel_name;
	server.reply(executor, join_msg);
}

// TODO: ERR_TOOMANYCHANNELS and ERR_UNAVAILRESOURCE and ERR_BADCHANMASK ??
// TODO: ban handling : ERR_BANNEDFROMCHAN

void JoinCommand::execute(Client *executor, Server &server)
{
	if (_params.size() < 1)
	{
		server.reply(executor, ERR_NEEDMOREPARAMS(executor->getNickname(), "JOIN"));
		return;
	}
	if (!executor->isRegistered())
	{
		server.reply(executor, ERR_NOTREGISTERED(executor->getNickname()));
		return;
	}
	if (_params[0] == "0")
	{
		// Executor wants to leave all channels
		return;
	}
	std::vector<std::string> channel_names;
	std::vector<std::string> channel_keys;
	std::stringstream ss_1(_params[0]);
	std::string channel_name;
	std::string channel_key;
	char del = ',';
	while (getline(ss_1, channel_name, del))
		channel_names.push_back(channel_name);
	if (_params.size() > 2)
	{
		std::stringstream ss_2(_params[1]);
		while (getline(ss_2, channel_key, del))
			channel_keys.push_back(channel_key);
	}
	Channel *channel;
	for (size_t i = 0; i < channel_names.size(); ++i)
	{
		if (channel_names[i].empty() || (channel_names[i][0] != '#' && channel_names[i][0] != '&'))
		{
			server.reply(executor, ERR_NOSUCHCHANNEL(executor->getNickname(), channel_names[i]));
			Debug::print(INFO, "Invalid channel name: " + channel_names[i]);
			continue;
		}
		std::string chan_key;
		if (channel_keys.size() >= i + 1)
			chan_key = channel_keys[i];
		else
			chan_key = "";
		channel = server.getChannel(channel_names[i]);
		if (!channel) // Channel does not exist so we create it
		{
			if (!server.addChannel(channel_names[i]) || !(channel = server.getChannel(channel_names[i])))
			{
				Debug::print(ERROR, "JoinCommand::execute: Unable to create channel " + channel_names[i]);
				continue;
			}
			channel->addOperator(executor); // First user is operator
			join_message(executor, server, channel_names[i]);
			continue;
		}
		if (channel->isClientInChannel(executor))
			continue; // Already in channel : ignore?
		if (channel->getModes().is_invite_only)
		{
			server.reply(executor, ERR_INVITEONLYCHAN(executor->getNickname(), channel_names[i]));
			continue;
		}
		if (channel->isKeySet(channel->getModes()))
		{
			if (!channel->isKeySame(chan_key))
			{
				server.reply(executor, ERR_BADCHANNELKEY(executor->getNickname(), channel_names[i]));
				continue;
			}
			if (channel->getModes().is_limited && channel->getClients().size() >= channel->getUserLimit())
			{
				server.reply(executor, ERR_CHANNELISFULL(executor->getNickname(), channel_names[i]));
				continue;
			}
			channel->addClient(executor);
			join_message(executor, server, channel_names[i]);
			server.reply(executor, RPL_TOPIC(executor->getNickname(), channel_names[i], channel->getTopic()));
			continue;
		}
		channel->addClient(executor);
		join_message(executor, server, channel_names[i]);
		server.reply(executor, RPL_TOPIC(executor->getNickname(), channel_names[i], channel->getTopic()));
	}
}
