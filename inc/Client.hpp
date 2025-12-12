#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <ctime>
#include <stdint.h>

struct ClientModes
{
	bool is_invisible; //!< +i: Invisible mode
	bool is_operator;  //!< +o: Operator mode
};

/**
 * @brief A Client as a representation of a user in the IRC server.
 *
 */
class Client
{
	//= Variables =//
private:
	//= Attributes for Client =//
	std::string _nickname; //!< User's nickname
	std::string _username; //!< User's username (ident)
	std::string _realname; //!< User's real name
	ClientModes _mode;
	std::string _host;		//!< User's host/hostname (can be generated)
	std::time_t _last_seen; //!< Last seen time
	bool _password_correct; //!< Flag indicating if the user has provided the correct password
	bool _registered;		//!< Flag indicating if the user has provided all the auth info
	std::string _ip_address;		//!< User's IP address
	uint16_t _port;				//!< User's port number
public:
	//= Attributes for LocalUser =//
	int _fd;						 //!< File descriptor for the user's socket connection
	std::string rbuf;			 //!< Read buffer to store incoming data
	std::string wbuf;			 //!< Write buffer to store outgoing data
	bool hasTriggeredEPOLLOUT;   //!< Flag indicating if EPOLLOUT is triggered
	std::time_t last_ping;		 //!< Time of the last ping received
	std::time_t timeout;		 //!< Time when the user will timeout if no activity
	std::time_t connection_time; //!< Time when user connected to server

	//= Methods =//
private:
	Client();
	Client(Client const &other);
public:
	Client(int fd, const std::string &ip_str, uint16_t port);
	~Client();

	void clear();
	void printClientInfo();
	void printClientIRCInfo();
	void printClientSocketInfo();
	void printConnInfo();

	//= Methods for Client =//
	const std::string &getKey() const;
	void setNickname(const std::string &nickname);
	const std::string &getNickname() const;
	void setUsername(const std::string &username);
	const std::string &getUsername() const;
	void setRealname(const std::string &realname);
	const std::string &getRealname() const;
	void setHost(const std::string &host);
	const std::string &getHost() const;
	void setMode(const ClientModes mode);
	const ClientModes getMode() const;
	void setLastSeen(std::time_t last_seen);
	const std::time_t &getLastSeen() const;
	bool isPasswordCorrect() const;
	void setPasswordCorrect();
	bool isRegistered() const;
	void setRegistered();
	const std::string &getIp() const;
};

#endif // CLIENT_HPP
