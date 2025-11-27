#ifndef ACOMMAND_HPP
#define ACOMMAND_HPP

#include <iostream>
#include <vector>
#include "Client.hpp"
#include "NetworkState.hpp"
#include "Debug.hpp"

enum command_type
{
	INVITE,
	KICK,
	CHANGE_TOPIC,
	MODE,
	NICK,
	USER,
	PASS,
	JOIN,
	LEAVE,
	SEND_MESSAGE,
	PRIVATE_MESSAGE,
	LIST_CHANNELS,
	LIST_USERS,
	UNKNOWN
};

enum param_type
{
	BOOL_PARAM,
	INT_PARAM,
	STRING_PARAM
};

class ACommand
{
//= Variables =//
protected:
	std::vector<std::string> _params; //!< Parameters for the command

//= Methods =//
public:
	virtual ~ACommand() {}
	virtual void execute(Client* executor, NetworkState& network) = 0;
	virtual void print_params() const
	{
		std::cout << "Command parameters:" << std::endl;
		for (size_t i = 0; i < _params.size(); ++i)
		{
			std::cout << "Param " << i << ": " << _params[i] << std::endl;
		}
	}
};

#endif  // ACOMMAND_HPP
