#include "PongCommand.hpp"
#include "Server.hpp"
#include <sstream>

PongCommand::PongCommand(std::vector<std::string> params)
{
	_params = params;
}

void PongCommand::execute(Client* executor, Server& server)
{
	(void)server;
	//checker le timestamp ici
  std::time_t now = std::time(NULL);
  executor->_localClient->timeout = now + 5;
  executor->_localClient->last_ping = now;
  //debug

  std::stringstream ss;
  ss << "["<< _params[0] << ": " << _params[1] 
    << "] from client " << executor->_localClient->fd 
    << " received";

  Debug::print(DEBUG, ss.str());
  // std::cout << format_time() << " Pong from client " << executor->_localClient->fd << " recieved" << std::endl; 
}

