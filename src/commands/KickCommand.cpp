#include "KickCommand.hpp"
#include "Channel.hpp"
#include "Server.hpp"
#include <sstream>

KickCommand::KickCommand(std::vector<std::string> params)
{
	_params = params;
}

static std::vector<std::string> split_param(std::string params)
{
  std::vector<std::string> vec;
  std::istringstream iss(params);
  std::string token;
  
  while (std::getline(iss, token, ',')) {
      vec.push_back(token);
  }
  return vec;
}

/**
 * @brief Execute the Kick command.
 * Kick a user from a channel.
 * Ex: KICK #channel user :msg
 * The targeted user leaves the channel.
 *
 * @param[in,out] executor client executing the command
 * @param[in,out] server server instance
 */
void KickCommand::execute(Client* executor, Server& server)
{
	if (!executor->isRegistered())
	{
		server.reply(executor, ERR_NOTREGISTERED(executor->getNickname()));
		return;
	}
	if (_params.size() < 2)
	{
		server.reply(executor, ERR_NEEDMOREPARAMS(executor->getNickname(), "KICK"));
		return;
	}

	std::vector<std::string> chans = split_param(_params[0]);
	std::vector<std::string> users = split_param(_params[1]);
	std::string kick_msg = "";
	if (_params.size() == 3) {
		kick_msg = _params[2];
	}

	if (chans.size() != users.size()) {
		//BADCHANMASK ?
		return;
	}

	for (size_t i = 0; i < chans.size(); ++i) {
		Channel* chan = server.getChannel(chans[i]);
		if (!chan || (chans[i][0] != '#' && chans[i][0] != '&' && chans[i].size() < 2))
		{
			server.reply(executor, ERR_NOSUCHCHANNEL(executor->getNickname(), _params[i]));
			continue;
		}
		if (!chan->isClientInChannel(executor))
		{
			server.reply(executor, ERR_NOTONCHANNEL(executor->getNickname(), chans[i]));
			continue;
		}
		if (!chan->isClientOPChannel(executor)) // isClientOPChannel a casse ?
		{
			server.reply(executor, ERR_CHANOPRIVSNEEDED(executor->getNickname(), _params[i]));
			continue;
		}
		Client* target = server.getClient(users[i]);
		if (!target)
		{
			server.reply(executor, ERR_NOSUCHNICK(executor->getNickname(), users[i]));
			continue;
		}
		if (!chan->isClientInChannel(target))
		{
			server.reply(executor, ERR_USERNOTINCHANNEL(executor->getNickname(), users[i], chans[i]));
			continue;
		}
		std::stringstream ss;
		ss << ":" << executor->getNickname() << "!~" << executor->getUsername() << "@" << executor->getRealname() // realname et pas ip ?
			<< " KICK " << chan->getName() << " " << target->getNickname();
		if (kick_msg != "") {
			ss << " :" << kick_msg;
		}
		server.replyChannel(chan, ss.str());
		chan->removeClient(target);
	}
	return;
}

