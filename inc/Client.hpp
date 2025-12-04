#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <ctime>

struct LocalUser; // Forward declaration

struct ClientModes
{
	bool is_invisible;	//!< +i: Invisible mode
	bool is_operator;	//!< +o: Operator mode
	// Add other modes as needed
};

// TODO: Implement modes on Client

/**
 * @brief A Client as a representation of a user in the IRC server.
 * 
 */
class Client
{
//= Variables =//
private:
	std::string _key;			//!< User's key in map
//= Attributes for Client =//
public:
	std::string _nickname;		//!< User's nickname
	std::string _username;		//!< User's username (ident)
	std::string _realname;		//!< User's real name
	ClientModes _mode;
	std::string _host;			//!< User's host/hostname (can be generated)
	std::time_t _last_seen;		//!< Last seen time
	bool _password_correct;		//!< Flag indicating if the user has provided the correct password
	bool _registered;			//!< Flag indicating if the user has provided all the auth info
//= Attributes for LocalUser =//
	int fd;              //!< File descriptor for the user's socket connection
	std::string rbuf;    //!< Read buffer to store incoming data
	std::string wbuf;    //!< Write buffer to store outgoing data
	std::time_t last_ping;  //!< Time of the last ping received
	std::time_t timeout;  //!< Time when the user will timeout if no activity
	std::time_t connection_time; //!< Time when user connected to server 

//= Methods =//
public:
	Client();
	Client(Client const &other);
	~Client();

	void clear();

	void printClientInfo();

	void setKey(const std::string &key);
	const std::string& getKey() const;

	void setNickname(const std::string &nickname);
	const std::string& getNickname() const;

	void setUsername(const std::string &username);
	const std::string& getUsername() const;

	void setRealname(const std::string &realname);
	const std::string& getRealname() const;

	void setHost(const std::string &host);
	const std::string& getHost() const;

	void setMode(const ClientModes mode);
	const ClientModes getMode() const;

	void setLastSeen(std::time_t last_seen);
	const std::time_t& getLastSeen() const;
};

#endif // CLIENT_HPP
