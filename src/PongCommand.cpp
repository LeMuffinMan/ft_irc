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
  executor->timeout = now + 5;
  executor->last_ping = now;
  //debug

  std::stringstream ss;
  ss << "["<< _params[0] << ": " << _params[1] 
    << "] from client " << executor->fd 
    << " received";

  Debug::print(DEBUG, ss.str());
  // std::cout << format_time() << " Pong from client " << executor->fd << " recieved" << std::endl; 
}

