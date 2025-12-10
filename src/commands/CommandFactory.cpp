#include <sstream>

#include "CommandFactory.hpp"
#include "InviteCommand.hpp"
#include "JoinCommand.hpp"
#include "TopicCommand.hpp"
#include "PassCommand.hpp"
#include "NickCommand.hpp"
#include "KickCommand.hpp"
#include "UserCommand.hpp"
#include "PongCommand.hpp"
#include "PrivmsgCommand.hpp"
#include "ModeCommand.hpp"
#include "PartCommand.hpp"
#include "QuitCommand.hpp"
#include "TimeCommand.hpp"

CommandFactory::CommandFactory() {}
CommandFactory::~CommandFactory() {}

/**
 * @brief Extract the command name from a command line.
 * 
 * @param[in] line full command line as a string
 * @return std::string 
 */
std::string CommandFactory::get_command(const std::string &line)
{
	size_t pos = line.find(' ');
	if (pos == std::string::npos)
		return line;
	return line.substr(0, pos);
}

/**
 * @brief Extract parameters from a command line.
 * 
 * @param[in] line full command line as a string
 * @return std::vector<std::string> 
 */
std::vector<std::string> CommandFactory::get_params(const std::string &line)
{
	std::vector<std::string> params;
	std::string last_param;

	size_t pos = line.find(' ');

	if (pos == std::string::npos)
		return params;

	std::string remaining = line.substr(pos + 1);
	std::string after_colon;
	bool has_colon = remaining.find(':') != std::string::npos;
	if (has_colon)
	{
		size_t colon_pos = remaining.find(':');
		after_colon = remaining.substr(colon_pos + 1);
		remaining = remaining.substr(0, colon_pos);
	}
	std::stringstream ss(remaining);
	std::string param;
	char del = ' ';
	while (getline(ss, param, del))
		params.push_back(param);
	if (has_colon)
		params.push_back(after_colon);
	return params;
}

/**
 * @brief Find and create a command object from a command line.
 * 
 * @param[in] line full command line as a string
 * @return ACommand* pointer to the created command object, or NULL if command is invalid
 */
ACommand *CommandFactory::findAndCreateCommand(const std::string &line)
{
	// Debug::print(DEBUG, "Parsing command: [" + line + "]");
	std::string cmd = CommandFactory::get_command(line);
	if (cmd.empty())
		return NULL;
	std::vector<std::string> params = CommandFactory::get_params(line);
	return CommandFactory::createCommand(cmd, params);
}

/**
 * @brief Find the command type based on the command name.
 * 
 * @param[in] command_name command name as a string
 * @return command_type corresponding command type enum value
 */
command_type CommandFactory::findType(const std::string &command_name)
{
	if (command_name == "INVITE") return INVITE;
	if (command_name == "KICK") return KICK;
	if (command_name == "TOPIC") return TOPIC;
	if (command_name == "MODE") return MODE;
	if (command_name == "NICK") return NICK;
	if (command_name == "USER") return USER;
	if (command_name == "PASS") return PASS;
	if (command_name == "JOIN") return JOIN;
	if (command_name == "PRIVMSG") return PRIVATE_MESSAGE;
	if (command_name == "PONG") return PONG;
	if (command_name == "PART") return PART;
	if (command_name == "QUIT") return QUIT;
	if (command_name == "TIME") return TIME;
	return UNKNOWN;
}

/**
 * @brief Create a command object based on the command type.
 * 
 * @param[in] command command name as a string
 * @param[in] params parameters for the command
 * @return ACommand* pointer to the created command object, or NULL if unknown command
 */
ACommand *CommandFactory::createCommand(const std::string &command, const std::vector<std::string> &params)
{
	command_type cmdType = CommandFactory::findType(command);
	switch (cmdType)
	{
	case INVITE:
		return new InviteCommand(params);
	case TOPIC:
		return new TopicCommand(params);
	case JOIN:
		return new JoinCommand(params);
	case NICK:
		return new NickCommand(params);
	case KICK:
		return new KickCommand(params);
	case USER:
		return new UserCommand(params);
	case PASS:
		return new PassCommand(params);
	case PRIVATE_MESSAGE:
		return new PrivmsgCommand(params);
	case PONG:
		return new PongCommand(params);
	case MODE:
		return new ModeCommand(params);
	case PART:
		return new PartCommand(params);
	case QUIT:
		return new QuitCommand(params);
	case TIME:
		return new TimeCommand(params);
	case UNKNOWN:
	default:
		return NULL;
	}
}
