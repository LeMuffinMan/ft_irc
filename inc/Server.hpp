#ifndef SERVER_HPP
#define SERVER_HPP

#include <vector>
#include <string>
#include <map>
#include <ctime> 
#include <sys/epoll.h>
#include "NetworkState.hpp"
#include "ACommand.hpp"
#include <vector>
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

#define PING_INTERVAL 5
#define PING_TIMEOUT 3

const int MAX_EVENTS = 64;	//Faire une taille dynamique (au fil de l'eau -> vecteur)
							//Interet des bornes ? deinfe / global

class ACommand;

class Server {
private:
	int _port;
	std::vector<int> _localUsers_fd;
	std::string _password;
	std::map<int, LocalUser> _localUsers;
	int _server_socket;
	int _epfd;
	NetworkState *_networkState;

	Server(); // On ne veut pas de serveur sans mdp ni sans port
public:
	Server(int port, std::string password);
	~Server();
	std::string& getPassword();
	// int getPort() const;
	// int checkPassword();
	void RunServer();
	int init_network(NetworkState &networkState);
	int init_socket(int port);
	void init_localuser(int client_fd);
	int init_epoll_event(int client_fd);
	
	//I/O
	int read_client_fd(int fd);
	void enable_epollout(int fd);
	void disable_epollout(int fd);
	int write_client_fd(int fd);


	//Clients managing
	void handle_events(int n, epoll_event events[MAX_EVENTS]);
	void new_client(int server_fd);
	void client_quited(int fd); // leaved plutot que quited
	void send_welcome(int fd);
	void remove_inactive_localUsers();
	void check_localUsers_ping();

	//Parsing and execution of commands
	ACommand* parse_command(std::string line);
	std::string get_command(std::string line);
	std::vector<std::string> get_params(std::string line);
	void is_authentification_complete(int fd);
	void interpret_msg(int fd);
	NetworkState& getNetwork();

	//Reply
	bool reply(Client* client, std::string message);
	bool replyChannel(Channel* channel, std::string message);
	bool replyChannelOnlyOP(Channel* channel, std::string message);
	bool broadcast(NetworkState&, std::string message);
	bool noticeServers(NetworkState&, std::string message);

};

#endif	// SERVER_HPP
