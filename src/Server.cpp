#include <iomanip>
#include <sstream>
#include <ctime>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <cerrno>
#include <cstdlib>
#include <cstring>

#include "Server_utils.h"
#include "Server.hpp"
#include "ACommand.hpp"
#include "CommandFactory.hpp"

// #define USE_FULL_CLIENT
// #define USE_TESTER
#ifdef USE_TESTER
#define USE_FULL_CLIENT
#endif

Server::~Server()
{
	for (clientsIterator it = this->clients.begin(); it != this->clients.end(); ++it)
	{
		close(it->first);
	}
	for (channelsIterator it = this->channels.begin(); it != this->channels.end(); ++it)
	{
		delete it->second;
	}
}

Server::Server(int port, std::string password) : _port(port)
{
	g_sig = 0;
	_password = password;
	signal_init();
}

// To documentate
int Server::init_socket(int port)
{
	#ifdef SOCK_NONBLOCK
	this->_server_socket = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
	#else
	this->_server_socket = socket(AF_INET, SOCK_STREAM, 0);
	#endif
	if (this->_server_socket < 0)
	{
		perror("socket");
		return -1;
	}

	int opt = 1;
	if (setsockopt(this->_server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
	{
		perror("setsockopt");
		close(this->_server_socket);
		return -1;
	}

	sockaddr_in sin;
	::memset(&sin, 0, sizeof(sin));

	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = htonl(INADDR_ANY);

	sin.sin_port = htons(static_cast<uint16_t>(port));

	if (bind(this->_server_socket, reinterpret_cast<const sockaddr *>(&sin), sizeof(sin)) < 0)
	{
		perror("bind");
		close(this->_server_socket);
		return -1;
	}
	if (listen(this->_server_socket, SOMAXCONN) < 0)
	{
		perror("listen");
		close(this->_server_socket);
		return -1;
	}
	#ifndef SOCK_NONBLOCK
	if (make_nonblocking(this->_server_socket) < 0)
	{
		perror("make_nonblocking");
		close(this->_server_socket);
		return -1;
	}
	#endif
	return this->_server_socket;
}

// When we want to write in a socket, using this function will trigger epoll_wait
// and lead us to write_client_fd
void Server::enable_epollout(int fd)
{
	epoll_event ev;
	ev.events = EPOLLIN | EPOLLOUT | EPOLLRDHUP;
	ev.data.fd = fd;
	epoll_ctl(_epfd, EPOLL_CTL_MOD, fd, &ev);
}

// When we wrote in client fd, we don't want epoll_wait to be triggered to write again,
// we switch off the flag EPOLLOUT
void Server::disable_epollout(int fd)
{
	epoll_event ev;
	ev.events = EPOLLIN | EPOLLRDHUP;
	ev.data.fd = fd;
	epoll_ctl(_epfd, EPOLL_CTL_MOD, fd, &ev);
}

int Server::init_epoll_event(int client_fd)
{
	// each client registered in epoll_ctl must have an event struct associated
	epoll_event cev;
	std::memset(&cev, 0, sizeof(cev));
	// theses flags define what we want to trigger epoll_wait :
	// In case of data to read or
	// if client closed its writing end (fragmented msgs)
	cev.events = EPOLLIN | EPOLLRDHUP;
	// we bind this new client event struct, with the client fd
	cev.data.fd = client_fd;
	// adding our new fd, and the event struct to our epoll, with the events we just set
	if (epoll_ctl(this->_epfd, EPOLL_CTL_ADD, client_fd, &cev) < 0)
	{
		perror("epoll_ctl add client");
		close(client_fd);
		return 1;
	}
	return 0;
}

void Server::init_localuser(int client_fd, const std::string &ip_str, uint16_t port)
{
	// since we added a client in our epoll, we need a struct to represent it on our server
	// LocalUser contains the pipes and tools, Client contains its server infos
	Client c;
	// the fd makes the link between epoll and our list of client
	c.fd = client_fd;
	// for non blocking or overlap situations, we need 2 I/O buffers for each client
	// we want to kick incactives clients
	c.last_ping = std::time(NULL);
	c.connection_time = std::time(NULL);
	c.timeout = -1;
	c.ip_address = ip_str;
	c.port = port;
	// the client object contains
	this->clients.insert(std::make_pair(client_fd, c));
	std::stringstream ss;
	ss << "New client: " << client_fd;
	Debug::print(DEBUG, ss.str());
}

// for each call to accept with our server fd, if there is a client to register, it will returns a > 0 fd
// We register all clients available with the true loop, and break only in case of error, or when there is no client to register
void Server::new_client()
{
	while (true)
	{
		sockaddr_in client_addr;
		socklen_t client_len = sizeof(client_addr);
		int client_fd = accept(_server_socket, reinterpret_cast<sockaddr*>(&client_addr), &client_len);
		if (client_fd < 0)
		{
			if (errno == EAGAIN || errno == EWOULDBLOCK)
				break;
			if (errno == EMFILE || errno == ENFILE)
			{
				Debug::print(ERROR, "TOO MANY FILE DESCRIPTORS!");
			}
			perror("accept");
			break;
		}
		if (make_nonblocking(client_fd) < 0)	
		{
			perror("make_nonblocking client");
			close(client_fd);
			continue;
		}
		if (init_epoll_event(client_fd))
			continue;
		char ip_str[INET_ADDRSTRLEN];
		uint16_t port = ntohs(client_addr.sin_port);
		inet_ntop(AF_INET, &client_addr.sin_addr, ip_str, sizeof(ip_str));
		init_localuser(client_fd, ip_str, port);
		this->clients[client_fd].printClientSocketInfo();
	}
}

void Server::client_kicked(int fd)
{
	std::stringstream ss;
	ss << "Client kicked: " << fd;
	Debug::print(INFO, ss.str());
	this->removeClient(fd);
}

void Server::client_quited(int fd) // leaved plutot que quited
{
	std::stringstream ss;
	ss << "Client left: " << fd;
	Debug::print(INFO, ss.str());
	this->removeClient(fd);
}

void Server::removeClient(int fd)
{
	epoll_ctl(this->_epfd, EPOLL_CTL_DEL, fd, NULL);
	close(fd);
	this->clients.erase(fd);
	// this->clients[fd].printClientInfo();
}

void Server::removeClient(Client *client)
{
	if (!client)
		return;
	removeClient(client->fd);
}

std::string &Server::getPassword()
{
	return this->_password;
}

ACommand *Server::parse_command(std::string line)
{
	Debug::print(DEBUG, "Parsing command: [" + line + "]");
	std::string cmd = CommandFactory::get_command(line);
	if (cmd.empty())
		return NULL;
	std::vector<std::string> params = CommandFactory::get_params(line);
	return CommandFactory::createCommand(cmd, params);
}

// returns 1 to send a buff in parsing
// returns 0 in case of client disconnection
// returns -1 in case of error
int Server::read_client_fd(int fd)
{
	char buf[4096];

	// MSG_DONTWAIT : rend non bloquant et suffisant ?
	ssize_t r = recv(fd, buf, sizeof(buf), MSG_DONTWAIT);

	if (r >= 512)
	{
		// si on veut traiter les 512 premiers octets il faut read ici
		this->reply(&this->clients[fd], "Buffer limit reached, only 512 bytes including \\r\\n allowed");
		std::stringstream ss;
		ss << "Message " << r << " bytes long from " << fd << " ignored";
		Debug::print(INFO, ss.str());
		// std::cerr << "Buffer limit reached for fd " << fd << ", cleaning buffer" << std::endl;
		this->clients[fd].rbuf.clear();
		return 0; // 1 si on veut lire et traiter les 512 premiers octets
	}

	if (r > 0)
	{
		this->clients[fd].rbuf.append(buf, &buf[r]);
		// std::stringstream ss;
		// ss << "Received " << r << " bytes from fd " << fd << " [" << this->clients[fd].rbuf << "]";
		// Debug::print(DEBUG, ss.str());
		return 1;
	}
	else if (r == 0)
	{
		std::cout << "Client " << fd << " disconnected" << std::endl;
		epoll_ctl(this->_epfd, EPOLL_CTL_DEL, fd, NULL);
		close(fd);
		this->clients.erase(fd);
		return 0;
	}
	else
	{
		// error handling : ici il faut que le parsing attende !
		// Ces flags sont importants si on utilise EPOLET, sans, c'est peut etre superflu
		//  if (errno == EAGAIN || errno == EWOULDBLOCK) {
		//	   return 2; // 1 -> 2
		//  } else {
		perror("recv");
		epoll_ctl(this->_epfd, EPOLL_CTL_DEL, fd, NULL);
		close(fd);
		this->clients.erase(fd);
		return -1;
		// }
	}
}

// a mettre en bas de PASS USER et NICK uniquement
void Server::is_authentification_complete(int fd)
{
	if (clients.find(fd) != clients.end() &&
		!clients[fd].isRegistered() &&
		clients[fd].isPasswordCorrect() == true &&
		clients[fd].getNickname() != "" &&
		clients[fd].getUsername() != "")
	{

		Client &client = this->clients[fd];
		this->reply(&client, RPL_WELCOME(client.getNickname(), client.getUsername(), "127.0.0.1"));
		client.setRegistered();
		std::stringstream ss;
		ss << clients[fd].getUsername() << " aka " << clients[fd].getNickname() << " successfully connected";
		client.printClientIRCInfo();
		Debug::print(DEBUG, ss.str());
	}
}

void Server::interpret_msg(int fd)
{
	size_t pos;
	while ((pos = this->clients[fd].rbuf.find("\r\n")) != std::string::npos)
	{
		std::string line = this->clients[fd].rbuf.substr(0, pos);
		this->clients[fd].rbuf.erase(0, pos + 2);
		ACommand *cmd = this->parse_command(line);
		// try catch ?
		if (cmd)
		{
			cmd->execute(&this->clients[fd], *this);
			delete cmd;
			// this->clients[fd]->printClientInfo();
		}
		else
		{
			std::stringstream ss;
			ss << "[" << line << "]"
			   << " from client " << fd
			   << " received";

			Debug::print(INFO, ss.str());
		}
	}
	this->clients[fd].last_ping = std::time(NULL);
	this->is_authentification_complete(fd);
}

void Server::handle_events(int n, epoll_event events[MAX_EVENTS])
{
	// for each event received during epoll_wait
	for (int i = 0; i < n; ++i)
	{
		int fd = events[i].data.fd;
		uint32_t evs = events[i].events;

		if (fd == this->_server_socket)
		{
			this->new_client();
		}
		else
		{
			// HUP : fd closed by client : the socket is dead
			// ERR : Error on fdSer
			if (evs & (EPOLLHUP | EPOLLERR))
			{ // in case of EPOLLHUP / EPOLLRDHUP : we clean our map, but is there any other possibility of client leaving without saying ?
				// std::stringstream ss;
				// ss << "EPOLLERR/HUP on fd " << fd;
				// Debug::print(ERROR, ss.str());
				this->client_quited(fd);
				continue;
			}
			// RDHUP :  client closed fd, the socket is still alive
			if (evs & EPOLLRDHUP)
			{
				// std::stringstream ss;
				// ss << "EPOLLRDHUP on fd " << fd;
				// Debug::print(INFO, ss.str());
				this->client_quited(fd);
				continue;
			}
			// EPOLLOUT : We set that flag when we write in a client buffer, we need to send it
			if (evs & EPOLLOUT)
			{
				if (!this->reply(&this->clients[fd], "")) // empty message to trigger write of wbuf != true)
					continue;
			}
			// EPOLLIN : There is data to read in the fd associated
			if (evs & EPOLLIN)
			{
				int result = this->read_client_fd(fd);
				// 0 = client disconnected
				if (result == 0)
				{
					continue;
				}
				else if (result == 1)
				{
					interpret_msg(fd);
				}
				else if (result < 0)
				{
					// error handling
				}
			}
		}
	}
}

// revoir ici max event et la logique
int Server::RunServer()
{
	this->_server_socket = init_socket(this->_port);
	if (this->_server_socket < 0)
		return EXIT_FAILURE;
	std::stringstream ss;
	ss << "Listening on port: " << this->_port;
	Debug::print(INFO, ss.str());

	this->_epfd = init_epoll(this->_server_socket);
	// doc
	epoll_event ev;
	ev.events = EPOLLIN | EPOLLRDHUP; // RDHUP pour détecter fermeture distante
									  // EPOLLET permet de rendre les sockets non bloquants ?;
	ev.data.fd = this->_server_socket;
	if (epoll_ctl(this->_epfd, EPOLL_CTL_ADD, this->_server_socket, &ev) < 0)
	{
		perror("epoll_ctl add server");
		close(this->_server_socket);
		close(this->_epfd);
		return EXIT_FAILURE;
	}

	// hash map pour associer chaque client a son fd : acceder a chaque client en utilisant son fd comme cle
	epoll_event events[MAX_EVENTS];

	while (g_sig == 0)
	{
		// we check for events from our localUsers fd registered
		int n = epoll_wait(this->_epfd, events, MAX_EVENTS, 100); // timeout 100ms
		if (n < 0)
		{
			// if (errno == EINTR)
			//	 continue; // signal interrompt -> relancer
			perror("epoll_wait");
			break;
		}
		handle_events(n, events);
		deleteUnusedChannels();
		#ifdef USE_FULL_CLIENT
		this->check_clients_ping();		 // si on n'a pas eu de signe d'activite depuis trop longtemps
		this->remove_inactive_clients(); // remove inactive localUsers after a unanswered ping
		#endif
	}
	close(this->_server_socket);
	close(this->_epfd);
	return EXIT_SUCCESS;
}
