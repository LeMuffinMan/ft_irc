#include "Channel.hpp"
#include "Debug.hpp"
#include <cstring>

Channel::Channel(std::string channel_name) : _channel_name(channel_name), _topic(""), _key(""), _mode(), _user_limit(0), _clients(), _operators(), _allowed_clients(), _allowed_operators()
{
	std::memset(&this->_mode, 0, sizeof(ChannelModes));
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
	this->_allowed_operators.clear();
}

void Channel::setModes(ChannelModes modes)
{
	this->_mode = modes;
}

ChannelModes Channel::getModes() const
{
	return this->_mode;
}

bool Channel::isKeySet(const ChannelModes &modes) const
{
	return modes.has_key;
}

bool Channel::isKeySame(const std::string &key) const
{
	return this->_key == key;
}

void Channel::setKey(const std::string &key)
{
	this->_key = key;
}

void Channel::setTopic(const std::string &topic)
{
	this->_topic = topic;
}

const std::string &Channel::getTopic() const
{
	return this->_topic;
}

const std::string &Channel::getName() const
{
	return this->_channel_name;
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

bool Channel::removeClient(Client *client)
{
	if (isClientInChannel(client) == false)
		return false;
	_clients.erase(client);
	return true;
}

bool Channel::addClientToAllowList(Client *client)
{
	if (isClientInAllowList(client))
		return false;
	_allowed_clients.insert(client);
	return true;
}

bool Channel::removeClientFromAllowList(Client *client)
{
	if (isClientInAllowList(client))
		return false;
	_allowed_clients.erase(client);
	return true;
}

bool Channel::addOperator(Client *client)
{
	if (isClientOPChannel(client))
		return false;
	if (!isClientInChannel(client))
		this->addClient(client);
	_operators.insert(client);
	this->_allowed_operators.insert(client);
	return true;
}

bool Channel::removeOperator(Client *client)
{
	if (!isClientOPChannel(client))
		return false;
	_operators.erase(client);
	return true;
}

bool Channel::isClientInChannel(Client *client) const
{
	return _clients.find(client) != _clients.end();
}

bool Channel::isClientInAllowList(Client *client) const
{
	return _allowed_clients.find(client) != _allowed_clients.end();
}

bool Channel::isClientOPChannel(Client *client) const
{
	return _operators.find(client) != _operators.end();
}

Client *Channel::getClientByNickname(const std::string &nickname) const
{
	Debug::print(INFO, "Searching for client in channel" + this->_channel_name + "with nickname: " + nickname);
	for (clientsType::const_iterator it = this->_clients.begin(); it != this->_clients.end(); ++it)
	{
		if ((*it)->getNickname() == nickname)
			return *it;
	}
	Debug::print(INFO, "Searching for operators in channel" + this->_channel_name + "with nickname: " + nickname);
	for (operatorsType::const_iterator it = this->_operators.begin(); it != this->_operators.end(); ++it)
	{
		if ((*it)->getNickname() == nickname)
			return *it;
	}
	return NULL;
}

Channel::clientsType &Channel::getClients()
{
	return this->_clients;
}

Channel::operatorsType &Channel::getOperators()
{
	return this->_operators;
}

size_t Channel::getUserLimit() const
{
	return this->_user_limit;
}

void Channel::setUserLimit(size_t limit)
{
	this->_user_limit = limit;
}
