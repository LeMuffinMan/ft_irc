#include "Client.hpp"
#include <cstring>

Client::Client() : _localClient(0)
{
	this->clear();
}

Client::Client(int fd) : _localClient(0)
{
	this->clear();
	this->_localClient = new LocalUser;
	this->_localClient->fd = fd;
	this->_isLocal = true;
}

Client::Client(LocalUser *localClient) : _localClient(0)
{
	this->clear();
	this->_localClient = localClient;
	this->_isLocal = true;
}

Client::~Client()
{
}

void Client::clear()
{
	this->_nickname.clear();
	this->_username.clear();
	this->_realname.clear();
	this->_host.clear();
	memset(&this->_mode, 0, sizeof(ClientModes));
	this->_last_seen = 0;
	this->_password_correct = false;
	this->_registered = false;
	if (this->_localClient)
		delete this->_localClient;
}

void Client::setLocalClient(LocalUser *localClient)
{
	if (this->_localClient)
		delete this->_localClient;
	this->_localClient = localClient;
	this->_isLocal = true;
}

LocalUser* Client::getLocalClient()
{
	return this->_localClient;
}

void Client::setNickname(const std::string &nickname)
{
	this->_nickname = nickname;
}

const std::string& Client::getNickname() const
{
	return this->_nickname;
}

void Client::setUsername(const std::string &username)
{
	this->_username = username;
}

const std::string& Client::getUsername() const
{
	return this->_username;
}

void Client::setRealname(const std::string &realname)
{
	this->_realname = realname;
}

const std::string& Client::getRealname() const
{
	return this->_realname;
}

void Client::setHost(const std::string &host)
{
	this->_host = host;
}

const std::string& Client::getHost() const
{
	return this->_host;
}

void Client::setMode(const ClientModes mode)
{
	this->_mode = mode;
}

const ClientModes Client::getMode() const
{
	return this->_mode;
}

void Client::setLastSeen(std::time_t last_seen)
{
	this->_last_seen = last_seen;
}

const std::time_t& Client::getLastSeen() const
{
	return this->_last_seen;
}
