#ifndef LOCALUSER_HPP
#define LOCALUSER_HPP

#include "Client.hpp"

class Client;

/**
 * @brief A LocalUser structure to represent a user connected to the local server.
 * This structure is used to hold data for socket communication and user identification.
 * 
 */
struct LocalUser
{
	int fd;              //!< File descriptor for the user's socket connection
	Client *client;      //!< Pointer to the associated Client structure
	std::string rbuf;    //!< Read buffer to store incoming data
	std::string wbuf;    //!< Write buffer to store outgoing data
	std::time_t last_ping;  //!< Time of the last ping received
	std::time_t timeout;  //!< Time when the user will timeout if no activity
};

#endif // LOCALUSER_HPP
