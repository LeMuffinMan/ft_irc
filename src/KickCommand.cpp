#include "NickCommand.hpp"
#include "Server.hpp"

KickCommand::KickCommand(std::vector<std::string> params)
{
	_params = params;
}

void KickCommand::execute(Client* executor, NetworkState& network, Server& server)
{
	std::vector<int> vec;

	(void)server;
	if (_params.size() < 3)
	{
		vec.push_back(461); // ERR_NEEDMOREPARAMS
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
			kick_msg = _params[1];
			break;
		}
		users.push_back(_params[i]);
	}
	if (users.size() != chans.size())
	{
		vec.push_back(476); //ERR_BADCHANMASK;
		return;
	}

	for (size_t i = 0; i < users.size(); ++i) {
		Channel* chan = network->getChannel(chans[i]);
		if (!chan)
		{
			vec.push_back(403); // no such channel
			continue;
		}
		if (!chan->IsClientInChannel(executor->getNickname()))
		{
			vec.push_back(442); //ERR_NOTONCHANNEL
			continue;
		}
		if (!chan->IsClientOPChannel(executor->getNickname()))
		{
			vec.push_back(482); // ERR_CHANOPRIVSNEEDED (482)
			continue;
		}
		if (!chan->IsClientInChannel(users[i]))
		{
			vec.push_back(441); //ERR_USERNOTINCHANNEL
		}

	if (vec.empty())
		vec.push_back(0);
	return;
}

// KICK #a,#b John,Mark :Raison du kick
//
