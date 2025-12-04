#include "Client.hpp"
#include "Debug.hpp"
#include <cstring>
#include <sstream>

Client::Client() : _key(""), _nickname(""), _username(""), _realname(""), _host(""), _last_seen(0), _password_correct(false), _registered(false), fd(-1), rbuf(""), wbuf(""), last_ping(0), timeout(0), connection_time(0)
{

}

Client::Client(Client const &other) : _key(other._key), _nickname(other._nickname), _username(other._username), _realname(other._realname), _mode(other._mode), _host(other._host), _last_seen(other._last_seen), _password_correct(other._password_correct), _registered(other._registered), fd(other.fd), rbuf(other.rbuf), wbuf(other.wbuf), last_ping(other.last_ping), timeout(other.timeout), connection_time(other.connection_time)
{
	// Copy constructor
}

Client::~Client()
{
}

void Client::clear()
{
	this->_key.clear();
	this->_nickname.clear();
	this->_username.clear();
	this->_realname.clear();
	this->_host.clear();
	memset(&this->_mode, 0, sizeof(ClientModes));
	this->_last_seen = 0;
	this->_password_correct = false;
	this->_registered = false;
}

void Client::printClientInfo()
{
	std::stringstream ss;
	Debug::print(INFO, "Client Info:");
	Debug::print(INFO, "Key: " + this->_key);
	Debug::print(INFO, "Type: Local");
	ss << "\tFD: " << this->fd;
	Debug::print(INFO, ss.str());
	Debug::print(INFO, "Nickname: " + this->_nickname);
	Debug::print(INFO, "Username: " + this->_username);
	Debug::print(INFO, "Realname: " + this->_realname);
	Debug::print(INFO, "Host: " + this->_host);
	int mode_flags = 0;
	if (this->_mode.is_invisible)
		mode_flags |= 0x01;
	if (this->_mode.is_operator)
		mode_flags |= 0x02;
	ss << "Mode: " << mode_flags;
	Debug::print(INFO, ss.str());
	ss.str("");
	ss << "Last Seen: " << this->_last_seen;
	Debug::print(INFO, ss.str());
	Debug::print(INFO, "Password Correct: " + std::string(this->_password_correct ? "true" : "false"));
	Debug::print(INFO, "Registered: " + std::string(this->_registered ? "true" : "false"));
}


void Client::setKey(const std::string &key)
{
	this->_key = key;
}

const std::string& Client::getKey() const
{
	return this->_key;
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

bool Client::isPasswordCorrect() const
{
	return this->_password_correct;
}

void Client::setPasswordCorrect(bool correct)
{
	this->_password_correct = correct;
}

bool Client::isRegistered() const
{
	return this->_registered;
}

void Client::setRegistered(bool registered)
{
	this->_registered = registered;
}