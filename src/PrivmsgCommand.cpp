#include "PrivmsgCommand.hpp"
#include "Server.hpp"

PrivmsgCommand::PrivmsgCommand(std::vector<std::string> params)
{
	_params = params;
}

//supporter l;envoie a des channels, si operator ?
void PrivmsgCommand::execute(Client* executor, Server& server)
{
	std::vector<int> vec;
	(void)server;

	if (!executor->_registered)
		return;
	if (_params.size() > 2)
		vec.push_back(407); // ERR_TOOMANYTARGETS
	if (_params[0].empty())
  	vec.push_back(411); // ERR_NORECIPIENT
  if (_params[1].empty()) 
  {
  	vec.push_back(412); // ERR_NOTEXTTOSEND
  }

	if (vec.empty())
	{
		Client* target = server.getClient(_params[1]);
		if (target)
		{
			// reply ici
			// vec.push_back(0);
		} else {
			vec.push_back(401); // ERR_NOSUCHNICK
		}
	}
  return;
}

// 	
//       ERR_CANNOTSENDTOCHAN   -> implique qu'on puisse envoyer un privmsg a un chan ?         
	// ERR_NOTOPLEVEL -> masque de domaine mal forme ?
//       ERR_WILDTOPLEVEL -> en lien avec le masque aussi                
//       RPL_AWAY -> en lien avec un statut "away"
