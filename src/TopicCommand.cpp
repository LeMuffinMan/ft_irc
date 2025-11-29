#include "TopicCommand.hpp"

TopicCommand::TopicCommand(std::vector<std::string> params)
{
	_params = params;
}

void TopicCommand::execute(Client* executor, Server& server)
{
	if (_params.size() < 1) {
		server.reply(executor, ERR_NEEDMOREPARAMS(executor->getNickname(), "TOPIC"));
		return;
	}
	Channel* channel = server.getNetwork().getChannel(_params[0]);
	if (!channel) {
		server.reply(executor, ERR_NOSUCHCHANNEL(executor->getNickname(), _params[0]));
		return;
	}
	if (channel->isClientInChannel(executor) == false) {
		server.reply(executor, ERR_NOTONCHANNEL(executor->getNickname(), _params[0]));
		return;
	}
	if (_params.size() == 1)
	{
		if (channel->getTopic().empty()) {
			server.reply(executor, RPL_NOTOPIC(executor->getNickname(), _params[0]));
		} else {
			server.reply(executor, RPL_TOPIC(executor->getNickname(), _params[0], channel->getTopic()));
		}
		return;
	}
	std::string new_topic = _params[1];
	if (channel->getModes().is_topic_set_op_only && !channel->isClientOPChannel(executor)) {
		server.reply(executor, ERR_CHANOPRIVSNEEDED(executor->getNickname(), _params[0]));
		return;
	}
	channel->setTopic(new_topic);
	server.replyChannel(channel, RPL_TOPIC(executor->getNickname(), _params[0], new_topic));
}
