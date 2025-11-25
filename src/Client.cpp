#include "Client.hpp"
#include <cstring>

Client::Client()
{
   this->clear();
}

Client::Client(int fd)
{
   this->clear();
   this->localClient.fd = fd;
   this->isLocal = true;
}

// Client::Client(LocalUser *localClient)
// {
//    this->clear();
//    this->localClient = localClient;
//    this->isLocal = true;
// }

Client::~Client()
{
}

void Client::clear()
{
   this->nickname.clear();
   this->username.clear();
   this->realname.clear();
   this->host.clear();
   memset(&this->mode, 0, sizeof(ClientModes));
   this->last_seen = 0;
   this->password_correct = false;
   this->registered = false;
}

void Client::setLocalClient(LocalUser *localClient)
{
   this->localClient = *localClient;
   this->isLocal = true;
}

void Client::setNickname(const std::string &nickname)
{
   this->nickname = nickname;
}

const std::string& Client::getNickname() const
{
   return this->nickname;
}

void Client::setUsername(const std::string &username)
{
   this->username = username;
}

const std::string& Client::getUsername() const
{
   return this->username;
}

void Client::setRealname(const std::string &realname)
{
   this->realname = realname;
}

const std::string& Client::getRealname() const
{
   return this->realname;
}

void Client::setHost(const std::string &host)
{
   this->host = host;
}

const std::string& Client::getHost() const
{
   return this->host;
}

void Client::setMode(const ClientModes mode)
{
   this->mode = mode;
}

const ClientModes Client::getMode() const
{
   return this->mode;
}

void Client::setLastSeen(std::time_t last_seen)
{
   this->last_seen = last_seen;
}

const std::time_t& Client::getLastSeen() const
{
   return this->last_seen;
}
