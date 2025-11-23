#include "CommandFactory.hpp"
#include "InviteCommand.hpp"

CommandFactory::CommandFactory() {}
CommandFactory::~CommandFactory() {}

CommandFactory& CommandFactory::getInstance() {
	static CommandFactory instance;
	return instance;
}

command_type CommandFactory::findType(std::string const& command_name) {
    if (command_name == "INVITE") return INVITE;
    if (command_name == "KICK") return KICK;
	if (command_name == "CHANGE_TOPIC") return CHANGE_TOPIC;
	if (command_name == "MODE") return MODE;
	if (command_name == "NICK") return NICK;
	if (command_name == "USER") return USER;
	if (command_name == "PASS") return PASS;
	if (command_name == "JOIN") return JOIN;
	if (command_name == "LEAVE") return LEAVE;
	if (command_name == "SEND_MESSAGE") return SEND_MESSAGE;
	if (command_name == "PRIVATE_MESSAGE") return PRIVATE_MESSAGE;
	if (command_name == "LIST_CHANNELS") return LIST_CHANNELS;
	if (command_name == "LIST_USERS") return LIST_USERS;
    return UNKNOWN;
}

ACommand* CommandFactory::createCommand(const std::string& command, const std::vector<std::string> &params)
{
	command_type cmdType = CommandFactory::findType(command);
	switch (cmdType)
	{
		case INVITE:
			return new InviteCommand(params);
		// case KICK:
		// 	return new KickCommand(params);
		// case CHANGE_TOPIC:
		// 	return new ChangeTopicCommand(params);
		// case MODE:
		// 	return new ModeCommand(params);
		// case NICK:
		// 	return new SetNickCommand(params);
		// case USER:
		// 	return new SetUsernameCommand(params);
		// case PASS:
		// 	return new PasswordCommand(params);
		// case JOIN:
		// 	return new JoinCommand(params);
		// case LEAVE:
		// 	return new LeaveCommand(params);
		// case SEND_MESSAGE:
		// 	return new SendMessageCommand(params);
		// case PRIVATE_MESSAGE:
		// 	return new PrivateMessageCommand(params);
		// case LIST_CHANNELS:
		// 	return new ListChannelsCommand(params);
		// case LIST_USERS:
		// 	return new ListUsersCommand(params);
		case UNKNOWN:
		default:
			return 0;
	}
}
