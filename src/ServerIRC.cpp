#include "Server.hpp"

Client* Server::getClient(const std::string& nickname)
{
	for (std::map<int, Client>::iterator it = this->clients.begin(); it != this->clients.end(); ++it)
	{
		if (it->second.getNickname() == nickname)
			return &(it->second);
	}
	return NULL;
}

Channel* Server::getChannel(const std::string& channel_name)
{
	if (channel_name.empty() || channel_name[0] != '#') {
		return NULL;
	}
	std::map<std::string, Channel*>::iterator it = channels.find(channel_name);
	if (it != channels.end()) {
		return it->second;
	}
	return NULL;
}

// bool Server::addClient(const std::string &nickname, Client *client)
// {
// 	Debug::print(DEBUG, "Adding client with nickname: " + nickname);
// 	std::map<std::string, Client*>::iterator it = clients.find(nickname);
// 	if (it == clients.end()) {
// 		clients[nickname] = client;
// 		return true;
// 	}
// 	return false;
// }

// bool Server::removeClient(const std::string &nickname)
// {
// 	std::map<std::string, Client*>::iterator it = clients.find(nickname);
// 	if (it != clients.end()) {
// 		delete it->second;
// 		clients.erase(it);
// 		return true;
// 	}
// 	return false;
// }

// Client* Server::getClient(const std::string &nickname)
// {
// 	std::map<std::string, Client*>::iterator it = clients.find(nickname);
// 	if (it != clients.end()) {
// 		return it->second;
// 	}
// 	return NULL;
// }

bool Server::addChannel(const std::string &channel_name)
{
	std::map<std::string, Channel*>::iterator it = channels.find(channel_name);
	if (it == channels.end()) {
		channels[channel_name] = new Channel(channel_name);
		return true;
	}
	return false;
}
