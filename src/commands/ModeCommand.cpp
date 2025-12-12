#include "ModeCommand.hpp"
#include <sstream>

ModeCommand::ModeCommand(std::vector<std::string> params)
{
	_params = params;
}

/**
 * @brief Execute the MODE command.
 * Either changes user modes or channel modes based on the parameters.
 * Ex: MODE #channel +o user -> channel mode change
 * Ex: MODE user +i -> user mode change
 *
 * @param[in,out] executor client executing the command
 * @param[in,out] server server instance
 */
void ModeCommand::execute(Client *executor, Server &server)
{
	if (!executor->isRegistered())
	{
		server.reply(executor, ERR_NOTREGISTERED(executor->getNickname()));
		return;
	}
	if (_params.size() < 2)
	{
		server.reply(executor, ERR_NEEDMOREPARAMS(executor->getNickname(), "MODE"));
		return;
	}
	if (!executor->isRegistered())
	{
		server.reply(executor, ERR_NOTREGISTERED(executor->getNickname()));
		return;
	}
	if (_params[0][0] != '#' && _params[0][0] != '&') // Param is not a channel
		executeUserMode(executor, server);
	else
		executeChannelMode(executor, server);
}

/**
 * @brief Execute user mode changes.
 * Ex: MODE user2 +i
 * on user user2, add invisible mode
 * Current supported modes:
 * none
 *
 * @param[in,out] executor client executing the command
 * @param[in,out] server server instance
 */
void ModeCommand::executeUserMode(Client *executor, Server &server)
{
	Client *target = server.getClient(_params[0]);
	if (target != executor)
	{
		server.reply(executor, ERR_USERSDONTMATCH(executor->getNickname()));
		return;
	}
	if (!target)
	{
		server.reply(executor, ERR_NOSUCHNICK(executor->getNickname(), _params[0]));
		return;
	}
	// TODO: not explicitly required by subject !!
}
				// :salut!~t@46.231.218.157 MODE #UNBONTEST +i 

/**
 * @brief Execute channel mode changes.
 * Change the modes of a channel.
 * Ex: MODE #channel +o user
 * on channel #channel, add operator status to user
 * Current supported modes:
 * +o : operator status
 * +i : invite only
 * +t : topic set by operator only
 * +k : key (password)
 * +l : user limit
 *
 * Input can be :
 * MODE #channel +o user1 -i +k key1 -l 10
 *
 * @param[in,out] executor client executing the command
 * @param[in,out] server server instance
 */
void ModeCommand::executeChannelMode(Client *executor, Server &server)
{
	Channel *channel = server.getChannel(_params[0]);
	if (!channel)
	{
		server.reply(executor, ERR_NOSUCHCHANNEL(executor->getNickname(), _params[0]));
		return;
	}
	if (!channel->isClientOPChannel(executor))
	{
		server.reply(executor, ERR_CHANOPRIVSNEEDED(executor->getNickname(), _params[0]));
		return;
	}
	size_t _paramSize = _params.size();
	for (size_t i = 1; i < _paramSize; ++i)
	{
		if (_params[i][0] != '+' && _params[i][0] != '-') // invalid mode format
			continue;
		if (_params[i][1] == 'o')
		{
			if (i + 1 >= _params.size())
			{
				server.reply(executor, ERR_NEEDMOREPARAMS(executor->getNickname(), "MODE"));
				continue;
			}
			if (!channel->isClientInChannel(server.getClient(_params[i + 1])))
			{
				server.reply(executor, ERR_USERNOTINCHANNEL(executor->getNickname(), _params[i + 1], _params[0]));
				continue;
			}
			Client *target = channel->getClientByNickname(_params[i + 1]);
			if (_params[i][0] == '+')
			{
				server.replyChannel(channel, RPL_CHANNELMODEIS(executor->getHost(), _params[0], "+o", _params[i + 1]));
				channel->addOperator(target);
			}
			else
			{
				server.replyChannel(channel, RPL_CHANNELMODEIS(executor->getHost(), _params[0], "-o", _params[i + 1]));
				channel->removeOperator(target); // retirer même si c'est le dernier ?? -> channel sans operateur
			}
		}
		else if (_params[i][1] == 'i')
		{
			ChannelModes modes = channel->getModes();
			if (_params[i][0] == '+')
			{
				server.replyChannel(channel, RPL_CHANNELMODEIS(executor->getHost(), _params[0], "+i", ""));
				modes.is_invite_only = true;
			}
			else
			{
				server.replyChannel(channel, RPL_CHANNELMODEIS(executor->getHost(), _params[0], "-i", ""));
				modes.is_invite_only = false;
			}
			channel->setModes(modes);
		}
		else if (_params[i][1] == 't')
		{
			ChannelModes modes = channel->getModes();
			if (_params[i][0] == '+')
			{
				server.replyChannel(channel, RPL_CHANNELMODEIS(executor->getHost(), _params[0], "+t", ""));
				modes.is_topic_set_op_only = true;
			}
			else
			{
				server.replyChannel(channel, RPL_CHANNELMODEIS(executor->getHost(), _params[0], "-t", ""));
				modes.is_topic_set_op_only = false;
			}
			channel->setModes(modes);
		}
		else if (_params[i][1] == 'k')
		{
			if (_paramSize < i + 1) // s'il n'y a pas de clé, on ne peut pas modifier
				continue;
			ChannelModes modes = channel->getModes();
			if (_params[i][0] == '+')
			{
				server.replyChannel(channel, RPL_CHANNELMODEIS(executor->getNickname(), _params[0], "+k", _params[i + 1]));
				modes.has_key = true;
				channel->setKey(_params[i + 1]);
			}
			else
			{
				if (channel->isKeySame(_params[i + 1]) == false)
					continue;
				server.replyChannel(channel, RPL_CHANNELMODEIS(executor->getNickname(), _params[0], "-k", "*"));
				modes.has_key = false;
				channel->setKey("*");
			}
			channel->setModes(modes);
			i++;
		}
		else if (_params[i][1] == 'l')
		{
			ChannelModes modes = channel->getModes();
			if (_params[i][0] == '+')
			{
				if (_paramSize < i + 1) // s'il n'y a pas de limite, on ne peut pas modifier
					continue;
					server.replyChannel(channel, RPL_CHANNELMODEIS(executor->getNickname(), _params[0], "+l", _params[i + 1]));
				modes.is_limited = true;
				std::istringstream iss(_params[i + 1]);
				size_t limit;
				iss >> limit;
				channel->setUserLimit(limit);
				i++;
			}
			else
			{
				server.replyChannel(channel, RPL_CHANNELMODEIS(executor->getNickname(), _params[0], "-l", ""));
				modes.is_limited = false;
			}
			channel->setModes(modes);
		}
	}
}
