#ifndef TOPIC_COMMAND_HPP
#define TOPIC_COMMAND_HPP

#include "ACommand.hpp"

class TopicCommand : public ACommand
{
//= Variables =//
protected:

//= Methods =//
public:
	TopicCommand(std::vector<std::string> params);
	~TopicCommand() {}
	void execute(Client* executor, Server& server);
};

#endif // TOPIC_COMMAND_HPP
