#include "Channel.hpp"
#include "Debug.hpp"
#include <cstring>

Channel::Channel(std::string channel_name)
{
	this->channel_name = channel_name;
}

Channel::~Channel()
{
	clear();
}

void Channel::clear()
{
	Debug::print(INFO, "Clearing channel " + this->channel_name);
	this->topic.clear();
	this->key.clear();
	std::memset(&this->mode, 0, sizeof(ChannelModes));
	this->user_limit = 0;
	this->clients.clear();
	this->operators.clear();
	this->allowed_clients.clear();
	this->operators_realnames.clear();
}

void Channel::setModes(ChannelModes modes)
{
	this->mode = modes;
}

ChannelModes Channel::getModes() const
{
	return this->mode;
}


bool Channel::isKeySet(const ChannelModes &modes)
{
	return modes.has_key;
}

bool Channel::isKeySame(const std::string &key)
{
	return this->key == key;
}

void Channel::setTopic(const std::string &topic)
{
	this->topic = topic;
}

const std::string& Channel::getTopic() const
{
	return this->topic;
}

bool Channel::addClient(Client *client)
{
	if (mode.is_limited && clients.size() >= user_limit)
		return false;
	if (isClientInChannel(client))
		return false;
	clients.insert(client);
	return true;
}

bool Channel::addClient(Client *client, bool is_operator)
{
	if (!is_operator && mode.is_limited && clients.size() >= user_limit)
		return false;
	if (isClientInChannel(client))
		return false;
	clients.insert(client);
	if (is_operator)
	{
		operators.insert(client);
		this->operators_realnames.push_back(client->realname);
	}
	return true;
}

bool Channel::removeClient(Client *client)
{
	if (isClientInChannel(client) == false)
		return false;
	clients.erase(client);
	return true;
}

bool Channel::isClientInChannel(Client *client) const
{
	return clients.find(client) != clients.end();
}

Client* Channel::getClientByNickname(const std::string &nickname) const
{
	Debug::print(INFO, "Searching for client in channel" + this->channel_name + "with nickname: " + nickname);
	for (std::set<Client*>::const_iterator it = this->clients.begin(); it != this->clients.end(); ++it)
	{
		if ((*it)->getNickname() == nickname)
			return *it;
	}
	Debug::print(INFO, "Searching for operators in channel" + this->channel_name + "with nickname: " + nickname);
	for (std::set<Client*>::const_iterator it = this->operators.begin(); it != this->operators.end(); ++it)
	{
		if ((*it)->getNickname() == nickname)
			return *it;
	}
	return NULL;
}
