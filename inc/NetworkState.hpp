#ifndef NETWORKSTATE_HPP
#define NETWORKSTATE_HPP

#include "Client.hpp"
#include <map>

// TODO: Conceptualize Channels

/** 
 * @brief Represents the state of the network, including all connected clients. 
 */
class NetworkState
{
//= Variables =//
private:
	std::map<std::string, Client*> clients; // Map of nickname to Client pointers

//= Methods =//
public:
	NetworkState();
	~NetworkState();

	void addClient(const std::string &nickname, Client *client);
	void removeClient(const std::string &nickname);
	Client *getClient(const std::string &nickname);
};

#endif // NETWORKSTATE_HPP
