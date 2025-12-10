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
	PRIVATE_MESSAGE,
	PONG,
	PART,
	QUIT,
	TIME,
	UNKNOWN
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
