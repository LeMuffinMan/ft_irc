#ifndef SERVER_HPP
#define SERVER_HPP

#include <map>
#include <cstdio>
#include <csignal>
#include <sys/epoll.h>
#include <netinet/in.h>

#include "ACommand.hpp"
#include "Client.hpp"
#include "Channel.hpp"
#include "Server_utils.h"

const int MAX_EVENTS = 64; // Faire une taille dynamique (au fil de l'eau -> vecteur)

//= Intervals in seconds =//
#define FLUSH_CHANNEL_INTERVAL 5
#define PING_INTERVAL 5
#define PING_TIMEOUT 3

class ACommand;
extern sig_atomic_t g_sig;

int signal_init(void);
void reset_signal_default(void);
void ignore_signal(void);

class Server
{
private:
	int _port;															 //!< Port number for the server
	std::string _password;												 //!< Password for the server
	std::map<int, Client> clients;										 //!< Map of client socket to Client class
	int _server_socket;													 //!< Server socket file descriptor
	int _epfd;															 //!< Epoll file descriptor
	std::map<std::string, Channel *> channels;							 //!< Map of channel name to Channel pointers
	typedef std::map<int, Client>::iterator clientsIterator;			 //!< Iterator for clients map
	typedef std::map<std::string, Channel *>::iterator channelsIterator; //!< Iterator for channels map

public:
	Server(int port, std::string password);
	~Server();
	std::string &getPassword();
	int RunServer();
	int init_socket(int port);
	void init_localuser(int client_fd, const std::string &ip_str, uint16_t port);
	int init_epoll_event(int client_fd);

	// I/O
	int read_client_fd(int fd);
	void enable_epollout(int fd);
	void disable_epollout(int fd);

	// Clients managing
	void client_kicked(int fd);
	void handle_events(int n, epoll_event events[MAX_EVENTS]);
	void new_client();
	void removeClient(int fd);
	void removeClient(Client *client);
	void client_quited(int fd);
	void remove_inactive_clients();
	void check_clients_ping();
	Client *getClient(const std::string &nickname);
	Channel *getChannel(const std::string &nickname);
	bool addChannel(const std::string &channel_name);
	bool removeChannel(const std::string &channel_name);
	void deleteUnusedChannels();

	// Parsing and execution of commands
	ACommand *parse_command(std::string line);
	void is_authentification_complete(int fd);
	void interpret_msg(int fd);

	// Reply
	bool reply(Client *client, std::string message);
	bool replyChannel(Channel *channel, std::string message);
	bool replyChannelOnlyOP(Channel *channel, std::string message);
};

#endif // SERVER_HPP
