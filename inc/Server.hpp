#ifndef SERVER_HPP
#define SERVER_HPP

#include <vector>
#include <string>
#include <map>
#include <ctime> 
#include <sys/epoll.h>
#include "NetworkState.hpp"

#include <sys/types.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <iostream>
#include <vector>
#include "Server.hpp"
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <cstdio>
#include <cstring>
#include <string.h>
#include <map> 
#include "Server_utils.h"

const int MAX_EVENTS = 64;	//Faire une taille dynamique (au fil de l'eau -> vecteur)
							//Interet des bornes ? deinfe / global

class Server {
private:
	int _port;		//!< Port number the server listens on
	std::vector<int> _localUsers_fd;		//!< Vector of file descriptors for local users
	const std::string _password;		//!< Server password for client authentication
	std::map<int, LocalUser> _localUsers;	//!< Map of file descriptors to LocalUser structures
	int _server_socket;		//!< Server socket file descriptor
	int _epfd;		//!< Epoll file descriptor
	NetworkState *_networkState;		//!< Pointer to the network state

	Server(); // On ne veut pas de serveur sans mdp ni sans port
public:
	// int getPort() const;
	// int checkPassword();
	void RunServer();
	int init_network(NetworkState &networkState);
	int init_socket(int port);
	int read_client_fd(int fd);
	void handle_events(int n, epoll_event events[MAX_EVENTS]);
	void send_welcome(int fd);
	void remove_inactive_localUsers();
	void check_localUsers_ping();
	void enable_epollout(int fd);
	void disable_epollout(int fd);
	int write_client_fd(int fd);
	void new_client(int server_fd);
	void client_quited(int fd); // leaved plutot que quited

	Server(int port, std::string password);
	~Server();
	// Server(const Server& other);
	// Server& operator=(const Server& other);
	bool reply(Client* client, std::string message);
	bool replyChannel(Channel& channel, std::string message);
	bool replyChannelOnlyOP(Channel& channel, std::string message);
	bool broadcast(NetworkState&, std::string message);
	bool noticeServers(NetworkState&, std::string message);
};

#endif	// SERVER_HPP
