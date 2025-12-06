#ifndef ACOMMAND_HPP
#define ACOMMAND_HPP

#include <iostream>
#include <vector>
#include "Client.hpp"
#include "Server.hpp"
#include "Debug.hpp"
#include "Reply.hpp"

class Server;

enum command_type
{
	INVITE,
	KICK,
	TOPIC,
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
	PONG,
	PART,
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
	virtual ~ACommand();
	virtual void execute(Client *executor, Server &server) = 0;
	virtual void print_params() const;
};

#endif // ACOMMAND_HPP
