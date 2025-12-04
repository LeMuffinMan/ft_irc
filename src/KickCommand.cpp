#include "NickCommand.hpp"
#include "Server.hpp"

KickCommand::KickCommand(std::vector<std::string> params)
{
	_params = params;
}

void KickCommand::execute(Client* executor, NetworkState& network, Server& server)
{
	if (!executor->_registered)
		return;

	(void)server;
	if (_params.size() < 2)
	{
		server.reply(executor, ERR_NEEDMOREPARAMS(executor->_nickname, "KICK"));
		return;
	}

	std::vector<std::string> chans;
	std::vector<std::string> users;
	std::string kick_msg = "";
	int count;
	for (size_t i = 1; i < _params.size(); ++i) {
    if(_params[i][0] != '#')
    	break;
    chans.push_back(_params[i]);
    count++;
	}
	for (size_t i = count; i < _params.size(); ++i) {
		if (_params[i][0] == ':')
		{
			kick_msg = _params[i];
			break;
		}
		users.push_back(_params[i]);
	}
	for (size_t i = 0; i < users.size(); ++i) {
		Channel* chan = network->getChannel(chans[i]);
		if (!chan || chans[i].find('#') == chans.end() && (chans[i].find('&') == chans.end()))
		{
			server.reply(executor, ERR_NOSUCHCHANNEL(executor->_nickname, _params[i]));
			continue;
		}
		if (!chan->IsClientInChannel(executor->getNickname()))
		{
			server.reply(executor, ERR_NOSUCHCHANNEL(executor->_nickname, _params[i]));
			continue;
		}
		if (!chan->IsClientOPChannel(executor->getNickname()))
		{
			server.reply(executor, ERR_CHANOPRIVSNEEDED(executor->_nickname, _params[i]));
			continue;
		}
		if (!chan->IsClientInChannel(users[i]))
		{
			server.reply(executor, ERR_USERNOTINCHANNEL(executor->_nickname, users[i], chans[i]));
		}

	return;
}

// KICK #a,#b John,Mark :Raison du kick
//
