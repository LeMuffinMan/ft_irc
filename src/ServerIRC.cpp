#include <cerrno>
#include <sstream>
#include "Server.hpp"
#include "ACommand.hpp"
#include "CommandFactory.hpp"

// on mettrait pas le timestamp dans le debug aussi ?
bool Server::reply(Client *client, std::string message)
{
	if (!client)
	{
		Debug::print(ERROR, "No client given");
		return false;
	}
	std::string &wbuf = client->wbuf;
	if (!message.empty())
		wbuf.append(message).append("\r\n");
	while (!wbuf.empty())
	{
		ssize_t n = send(client->fd, wbuf.c_str(), wbuf.size(), 0);

		if (n > 0)
		{
			Debug::print(INFO, "Reply to " + client->getNickname() + ": " + wbuf.substr(0, static_cast<size_t>(n - 1)));
			wbuf.erase(0, static_cast<size_t>(n));
		}
		// socket buffer full
		else if (n == -1 && (errno == EAGAIN || errno == EWOULDBLOCK))
		{
			Debug::print(INFO, "The message couldn't be send in one try, retrying next time");
			enable_epollout(client->fd);
			return 0;
		}
		else
		{
			// error or client disconnected -> Un client qu'on a deja kick
			std::cerr << "[ERROR] Send error on fd " << client->fd << "\n";
			epoll_ctl(_epfd, EPOLL_CTL_DEL, client->fd, NULL);
			close(client->fd);
			clients.erase(client->fd);
			return false;
		}
	}
	return true;
}

bool Server::replyChannel(Channel *channel, std::string message)
{
	if (!channel)
	{
		Debug::print(ERROR, "No channel given");
		return false;
	}
	bool ret = true;
	for (std::set<Client *>::iterator it = channel->getClients().begin(); it != channel->getClients().end(); ++it)
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
	for (std::set<Client *>::iterator it = channel->getOperators().begin(); it != channel->getOperators().end(); ++it)
	{
		if (!reply(*it, message))
		{
			ret = false;
			Debug::print(ERROR, "The following message couldn't be properly send: " + message);
		}
	}
	return ret;
}

void Server::remove_inactive_clients()
{
	std::time_t now = std::time(NULL);
	std::vector<int> to_erase;
	for (std::map<int, Client>::iterator it = this->clients.begin();
		 it != this->clients.end(); it++)
	{
		// if (!it->second)
		// 	continue;
		Client &client = it->second;
		now = std::time(NULL);
		if ((client.timeout > 0 && now > client.timeout) ||
			(!client.isRegistered() && client.connection_time + 10 < now))
		{
			std::stringstream ss;
			if (client.isRegistered())
			{
				// ss << client->getUsername()
				// << " aka " << client->getNickname()
				// << " timed out\r\n";
				this->reply(&client, "timed out");
			}
			else
			{
				// ss << "Disconnected: timed out" << std::endl;
				this->reply(&client, "timed out");
			}
			to_erase.push_back(it->first);
		}
	}
	for (std::vector<int>::iterator it = to_erase.begin(); it != to_erase.end(); it++)
		removeLocalUser(*it);
}

void Server::check_clients_ping()
{
	for (std::map<int, Client>::iterator it = this->clients.begin();
		 it != this->clients.end(); ++it)
	{
		// if (!it->second)
		// 	continue;
		int fd = it->first;
		(void)fd;
		Client &client = it->second;

		std::time_t now = std::time(NULL);

		if (client.isRegistered() && now - client.last_ping > PING_INTERVAL)
		{
			std::stringstream ss;
			ss << "PING :" << now;
			this->reply(&client, ss.str());
			client.timeout = now + 3;
			client.last_ping = now;
			// ss.clear();
			// ss << "[PING :" << now << "] sent to client " << fd;
			// Debug::print(DEBUG, ss.str());
		}
	}
}

Client *Server::getClient(const std::string &nickname)
{
	for (std::map<int, Client>::iterator it = this->clients.begin(); it != this->clients.end(); ++it)
	{
		if (it->second.getNickname() == nickname)
			return &(it->second);
	}
	return NULL;
}

Channel *Server::getChannel(const std::string &channel_name)
{
	if (channel_name.empty() || channel_name[0] != '#')
	{
		return NULL;
	}
	std::map<std::string, Channel *>::iterator it = channels.find(channel_name);
	if (it != channels.end())
	{
		return it->second;
	}
	return NULL;
}

bool Server::addChannel(const std::string &channel_name)
{
	if (channel_name.empty() || channel_name[0] != '#')
		return false;
	std::map<std::string, Channel *>::iterator it = channels.find(channel_name);
	if (it == channels.end())
	{
		channels.insert(std::make_pair(channel_name, new Channel(channel_name)));
		return true;
	}
	return false;
}
