#ifndef NETWORKSTATE_HPP
#define NETWORKSTATE_HPP

#include "Client.hpp"
#include "Channel.hpp"
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
	std::map<std::string, Channel*> channels; // Map of channel name to Channel pointers

//= Methods =//
public:
	NetworkState();
	~NetworkState();

	bool addClient(const std::string &nickname, Client *client);
	bool removeClient(const std::string &nickname);
	Client* getClient(const std::string &nickname);
	bool addChannel(const std::string &channel_name);
	Channel* getChannel(const std::string &channel_name);
};

#endif // NETWORKSTATE_HPP
