#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <ctime>
#include "LocalUser.hpp"

struct LocalUser; // Forward declaration

struct ClientModes
{
	bool is_invisible;  //!< +i: Invisible mode
	bool is_operator;  //!< +o: Operator mode
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
	bool isLocal;            //!< Flag indicating if user is local
public:
	LocalUser localClient;		//!< Pointer to LocalUser if local
	std::string nickname;    //!< User's nickname
	std::string username;    //!< User's username (ident)
	std::string realname;    //!< User's real name
	std::string host;        //!< User's host/hostname (can be generated)
	ClientModes mode;
	std::time_t last_seen; 	//!< Last seen time
	bool password_correct; //!< Flag indicating if the user has provided the correct password
	bool registered; //!< Flag indicating if the user has provided all the auth info

//= Methods =//
public:
	Client();
	Client(int fd);
	~Client();
	// Client(LocalUser *localClient);

	void clear();
	void setLocalClient(LocalUser *localClient);
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
