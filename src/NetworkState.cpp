#include "NetworkState.hpp"

NetworkState::NetworkState()
{
}

NetworkState::~NetworkState()
{
	// Clean up all Client pointers
	for (std::map<std::string, Client*>::iterator it = clients.begin(); it != clients.end(); ++it) {
		delete it->second;
	}
	clients.clear();
}

bool NetworkState::addClient(const std::string &nickname, Client *client)
{
	if (clients.find(nickname) == clients.end()) {
		clients[nickname] = client;
		return true;
	}
	return false;
}

bool NetworkState::removeClient(const std::string &nickname)
{
	if (clients.find(nickname) != clients.end()) {
		delete clients[nickname];
		clients.erase(nickname);
		return true;
	}
	return false;
}

Client* NetworkState::getClient(const std::string &nickname)
{
	if (clients.find(nickname) != clients.end()) {
		return clients[nickname];
	}
	return NULL;
}

bool NetworkState::addChannel(const std::string &channel_name)
{
	if (channels.find(channel_name) == channels.end()) {
		channels[channel_name] = new Channel(channel_name);
		return true;
	}
	return false;
}

Channel* NetworkState::getChannel(const std::string &channel_name)
{
	if (channels.find(channel_name) != channels.end()) {
		return channels[channel_name];
	}
	return NULL;
}
