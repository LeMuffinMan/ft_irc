#include "Channel.hpp"
#include "Debug.hpp"
#include <cstring>

Channel::Channel(std::string channel_name)
{
	this->_channel_name = channel_name;
	clear();
}

Channel::~Channel()
{
	clear();
}

void Channel::clear()
{
	Debug::print(INFO, "Clearing channel: " + this->_channel_name);
	this->_topic.clear();
	this->_key.clear();
	std::memset(&this->_mode, 0, sizeof(ChannelModes));
	this->_user_limit = 0;
	this->_clients.clear();
	this->_operators.clear();
	this->_allowed_clients.clear();
	this->_operators_realnames.clear();
}

void Channel::setModes(ChannelModes modes)
{
	this->_mode = modes;
}

ChannelModes Channel::getModes() const
{
	return this->_mode;
}


bool Channel::isKeySet(const ChannelModes &modes)
{
	return modes.has_key;
}

bool Channel::isKeySame(const std::string &key)
{
	return this->_key == key;
}

void Channel::setTopic(const std::string &topic)
{
	this->_topic = topic;
}

const std::string& Channel::getTopic() const
{
	return this->_topic;
}

bool Channel::addClient(Client *client)
{
	if (_mode.is_limited && _clients.size() >= _user_limit)
		return false;
	if (isClientInChannel(client))
		return false;
	_clients.insert(client);
	return true;
}

bool Channel::addClient(Client *client, bool is_operator)
{
	if (!is_operator && _mode.is_limited && _clients.size() >= _user_limit)
		return false;
	if (isClientInChannel(client))
		return false;
	_clients.insert(client);
	if (is_operator)
	{
		_operators.insert(client);
		this->_operators_realnames.push_back(client->getRealname());
	}
	return true;
}

bool Channel::removeClient(Client *client)
{
	if (isClientInChannel(client) == false)
		return false;
	_clients.erase(client);
	return true;
}

bool Channel::isClientInChannel(Client *client) const
{
	return _clients.find(client) != _clients.end();
}

bool Channel::isClientOPChannel(Client *client) const
{
	return _operators.find(client) != _operators.end();
}

Client* Channel::getClientByNickname(const std::string &nickname) const
{
	Debug::print(INFO, "Searching for client in channel" + this->_channel_name + "with nickname: " + nickname);
	for (std::set<Client*>::const_iterator it = this->_clients.begin(); it != this->_clients.end(); ++it)
	{
		if ((*it)->getNickname() == nickname)
			return *it;
	}
	Debug::print(INFO, "Searching for operators in channel" + this->_channel_name + "with nickname: " + nickname);
	for (std::set<Client*>::const_iterator it = this->_operators.begin(); it != this->_operators.end(); ++it)
	{
		if ((*it)->getNickname() == nickname)
			return *it;
	}
	return NULL;
}

std::set<Client*>& Channel::getClients()
{
	return this->_clients;
}

std::set<Client*>& Channel::getOperators()
{
	return this->_operators;
}
