#include <cerrno>
#include <sstream>
#include "Server.hpp"
#include "ACommand.hpp"
#include "CommandFactory.hpp"

// on mettrait pas le timestamp dans le debug aussi ?
/**
 * @brief Send a message to a client by the server
 * If the message can't be sent in one try,
 * it will enable EPOLLOUT for the client's fd, and try to send the rest later
 *
 * @param[in,out] client client to send the message to
 * @param[in] message message to send
 * @return true everything sent properly
 * @return false error occurred
 */
bool Server::reply(Client *client, std::string message)
{
	if (!client)
	{
		Debug::print(ERROR, "No client given");
		return false;
	}
	if (client->hasTriggeredEPOLLOUT)
		disable_epollout(client->_fd);
	std::string &wbuf = client->wbuf;
	if (!message.empty())
		wbuf.append(message).append("\r\n");
	while (!wbuf.empty())
	{
		ssize_t n = send(client->_fd, wbuf.c_str(), wbuf.size(), 0);
		int error = errno;
		if (n > 0)
		{
			Debug::print(INFO, "Reply to " + client->getNickname() + ": " + wbuf.substr(0, static_cast<size_t>(n - 1)));
			wbuf.erase(0, static_cast<size_t>(n));
		}
		else if (n < 0)
		{
			if (error == EAGAIN || error == EWOULDBLOCK)
			{
				Debug::print(INFO, "The message couldn't be send in one try, retrying next time");
				enable_epollout(client->_fd);
				return 0;
			}
			else if (error == EPIPE)
			{
				Debug::print(WARNING, "SIGPIPE received while sending message to client " + client->getNickname());
				removeClient(client);
				return false;
			}
		}
		else // n == 0
		{
			// error as send shouldn't return 0 or client disconnected?
			std::stringstream ss;
			ss << "Disconnected: send error on fd(" << client->_fd << ") with errno(" << error << ")";
			Debug::print(ERROR, ss.str());
			removeClient(client);
			return false;
		}
	}
	return true;
}

/**
 * @brief Send a message to all clients in a channel
 *
 * @param[in,out] channel channel to send the message to
 * @param[in] message message to send
 * @return true everything sent properly to all clients
 * @return false error occurred
 */
bool Server::replyChannel(Channel *channel, std::string message)
{
	if (!channel)
	{
		Debug::print(ERROR, "No channel given");
		return false;
	}
	bool ret = true;
	for (Channel::clientsType::iterator it = channel->getClients().begin(); it != channel->getClients().end(); ++it)
	{
		if (!reply(*it, message))
		{
			ret = false;
			Debug::print(ERROR, "The following message couldn't be properly send: " + message);
		}
	}
	return ret;
}

bool Server::replyChannelOnlyOP(Channel *channel, std::string message)
{
	if (!channel)
	{
		Debug::print(ERROR, "No channel given");
		return false;
	}
	bool ret = true;
	for (Channel::clientsType::iterator it = channel->getOperators().begin(); it != channel->getOperators().end(); ++it)
	{
		if (!reply(*it, message))
		{
			ret = false;
			Debug::print(ERROR, "The following message couldn't be properly send: " + message);
		}
	}
	return ret;
}

/**
 * @brief Remove inactive clients from the server
 * 
 */
void Server::remove_inactive_clients()
{
	std::time_t now = std::time(NULL);
	std::vector<int> to_erase;
	for (clientsType::iterator it = this->clients.begin(); it != this->clients.end(); it++)
	{
		Client *client = it->second;
		now = std::time(NULL);
		if ((client->timeout > 0 && now > client->timeout) ||
			(!client->isRegistered() && client->connection_time + 10 < now))
		{
			std::stringstream ss;
			if (client->isRegistered())
			{
				ss << "Registered client " << client->getNickname() << " got timed out";
				Debug::print(INFO, ss.str());
				this->reply(client, "timed out");
			}
			else
			{
				if (clients.find(it->first) != clients.end() && clients[it->first]->_fd > 0) // fixed l'erreur sur timed out fd = -1
				{
					ss << "Unregistered client with fd: " << client->_fd << " got timed out";
					Debug::print(INFO, ss.str());
					this->reply(client, "timed out");
				}
			}
			to_erase.push_back(it->first);
		}
	}
	for (std::vector<int>::iterator it = to_erase.begin(); it != to_erase.end(); it++)
		removeClient(*it);
}

/**
 * @brief Check all clients for ping timeouts and send PING messages if needed
 * 
 */
void Server::check_clients_ping()
{
	for (clientsType::iterator it = this->clients.begin(); it != this->clients.end(); ++it)
	{
		// if (!it->second)
		// 	continue;
		int fd = it->first;
		(void)fd;
		Client *client = it->second;

		std::time_t now = std::time(NULL);

		if (client->isRegistered() && now - client->last_ping > PING_INTERVAL)
		{
			std::stringstream ss;
			ss << "PING :" << now;
			this->reply(client, ss.str());
			client->timeout = now + 3;
			client->last_ping = now;
			// ss.clear();
			// ss << "[PING :" << now << "] sent to client " << fd;
			// Debug::print(DEBUG, ss.str());
		}
	}
}

Client *Server::getClient(const std::string &nickname)
{
	for (clientsType::iterator it = this->clients.begin(); it != this->clients.end(); ++it)
	{
		if (it->second->getNickname() == nickname)
			return (it->second);
	}
	return NULL;
}

Channel *Server::getChannel(const std::string &channel_name)
{
	if (channel_name.empty() || channel_name[0] != '#')
		return NULL;
	channelsType::iterator it = channels.find(channel_name);
	if (it != channels.end())
		return it->second;
	return NULL;
}

bool Server::addChannel(const std::string &channel_name)
{
	if (channel_name.empty() || channel_name[0] != '#')
		return false;
	channelsType::iterator it = channels.find(channel_name);
	if (it == channels.end())
	{
		channels.insert(std::make_pair(channel_name, new Channel(channel_name)));
		return true;
	}
	return false;
}

/**
 * @brief Remove a client from all channels they are connected to
 * This is typically called when a client disconnects from the server
 * 
 * @param[in] client Client to remove from all channels 
 */
void Server::removeClientFromAllChannels(Client *client)
{
	if (!client)
		return;
	for (channelsType::iterator it = channels.begin(); it != channels.end(); ++it)
	{
		Channel *channel = it->second;
		if (channel->removeClient(client)) // if succeed, client was in channel and is now removed
		{
			this->reply(client, ":" + client->getNickname() + " PART " + channel->getName());
			Debug::print(INFO, "Client " + client->getNickname() + " removed from channel " + channel->getName());
		}
	}
}

bool Server::removeChannel(const std::string &channel_name)
{
	if (channel_name.empty() || channel_name[0] != '#')
		return false;
	channelsType::iterator it = channels.find(channel_name);
	if (it != channels.end())
	{
		delete it->second;
		channels.erase(it);
		return true;
	}
	return false;
}

/**
 * @brief Delete channels that have no clients connected every FLUSH_CHANNEL_INTERVAL seconds
 * No need to send any notification, as channels are recreated when needed
 */
void Server::deleteUnusedChannels()
{
	static std::time_t last = std::time(NULL);
	std::time_t now = std::time(NULL);

	if (now - last < FLUSH_CHANNEL_INTERVAL)
		return;
	last = now;
	std::vector<std::string> to_delete;
	for (channelsType::iterator it = channels.begin(); it != channels.end(); ++it)
	{
		Channel *channel = it->second;
		if (channel->getClients().empty())
		{
			to_delete.push_back(it->first);
		}
	}
	for (size_t i = 0; i < to_delete.size(); ++i)
	{
		Debug::print(INFO, "Deleting unused channel: " + to_delete[i]);
		removeChannel(to_delete[i]);
	}
}
