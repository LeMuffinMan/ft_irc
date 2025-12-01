#include "NetworkState.hpp"
#include "Debug.hpp"
#include <iostream>

NetworkState::NetworkState()
{
}
NetworkState::~NetworkState()
{
	// Clean up all Client pointers
	// std::cout << "Number of clients to delete: " << clients.size() << std::endl;
	for (std::map<std::string, Client*>::iterator it = clients.begin(); it != clients.end(); ++it) {
		// std::cout << "Deleting client: " << it->second->getNickname() << std::endl;
		delete it->second;
	}
	// clients.clear();
	// Clean up all Channel pointers
	for (std::map<std::string, Channel*>::iterator it = channels.begin(); it != channels.end(); ++it) {
		delete it->second;
	}
	channels.clear();
}

bool NetworkState::addClient(const std::string &nickname, Client *client)
{
	Debug::print(DEBUG, "Adding client with nickname: " + nickname);
	std::map<std::string, Client*>::iterator it = clients.find(nickname);
	if (it == clients.end()) {
		clients[nickname] = client;
		return true;
	}
	return false;
}

bool NetworkState::removeClient(const std::string &nickname)
{
	std::map<std::string, Client*>::iterator it = clients.find(nickname);
	if (it != clients.end()) {
		delete it->second;
		clients.erase(it);
		return true;
	}
	return false;
}

Client* NetworkState::getClient(const std::string &nickname)
{
	std::map<std::string, Client*>::iterator it = clients.find(nickname);
	if (it != clients.end()) {
		return it->second;
	}
	return NULL;
}

bool NetworkState::addChannel(const std::string &channel_name)
{
	std::map<std::string, Channel*>::iterator it = channels.find(channel_name);
	if (it == channels.end()) {
		channels[channel_name] = new Channel(channel_name);
		return true;
	}
	return false;
}

Channel* NetworkState::getChannel(const std::string &channel_name)
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
