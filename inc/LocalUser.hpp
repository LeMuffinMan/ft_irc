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
	int fd;              // File descriptor for the user's socket connection
	bool registered;   // Registration status of the user
	Client *client;      // Pointer to the associated Client structure
	std::string rbuf;    // Read buffer to store incoming data
	std::string wbuf;    // Write buffer to store outgoing data
};

#endif // LOCALUSER_HPP
