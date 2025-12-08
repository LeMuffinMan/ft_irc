#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <string>
#include <ctime>
#include <set>
#include <vector>
#include "Client.hpp"

struct ChannelModes
{
	bool is_invite_only;	   //!< +i: Invite only mode
	bool is_topic_set_op_only; //!< +t: Topic set by operator only
	bool has_key;			   //!< +k: Key (password) set
	bool is_operator;		   //!< +o: Operator mode
	bool is_limited;		   //!< +l: User limit set
							   // Add other modes as needed
};

/**
 * @brief A Channel as a representation of a channel in the IRC server.
 *
 */
class Channel
{
	//= Variables =//
public:
	typedef std::set<Client *> clientsType;	  //!< Type for clients set
	typedef std::set<Client *> operatorsType; //!< Type for clients set
private:
	std::string _channel_name; //!< Channel's name
	std::string _topic;		   //!< Channel's topic
	std::string _key;		   //!< Channel's key (password)
	ChannelModes _mode;
	size_t _user_limit;				  //!< Maximum number of users allowed in the channel
	clientsType _clients;			  //!< Set of current Client pointers
	operatorsType _operators;		  //!< Set of current channel operators pointers
	clientsType _allowed_clients;	  //!< List of allowed clients
	operatorsType _allowed_operators; //!< List of allowed operators

	//= Methods =//
public:
	Channel(std::string channel_name);
	~Channel();

	void clear();
	void setModes(ChannelModes modes);
	ChannelModes getModes() const;
	void setKey(const std::string &key);
	bool isKeySet(const ChannelModes &modes) const;
	bool isKeySame(const std::string &key) const;
	void setTopic(const std::string &topic);
	const std::string &getTopic() const;
	const std::string &getName() const;

	bool addClient(Client *client);
	bool removeClient(Client *client);
	bool addClientToAllowList(Client *client);
	bool removeClientFromAllowList(Client *client);
	bool addOperator(Client *client);
	bool removeOperator(Client *client);
	bool isClientInChannel(Client *client) const;
	bool isClientInAllowList(Client *client) const;
	bool isClientOPChannel(Client *client) const;
	Client *getClientByNickname(const std::string &nickname) const;
	std::set<Client *> &getClients();
	std::set<Client *> &getOperators();
	size_t getUserLimit() const;
	void setUserLimit(size_t limit);
};

#endif // CHANNEL_HPP
