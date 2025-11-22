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

void NetworkState::addClient(const std::string &nickname, Client *client)
{
	if (clients.find(nickname) == clients.end()) {
		clients[nickname] = client;
	}
}

void NetworkState::removeClient(const std::string &nickname)
{
	if (clients.find(nickname) != clients.end()) {
		delete clients[nickname];
		clients.erase(nickname);
	}
}

Client *NetworkState::getClient(const std::string &nickname)
{
	if (clients.find(nickname) != clients.end()) {
		return clients[nickname];
	}
	return NULL;
}
