#include <iomanip>
#include <sstream>
#include <ctime>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>

#include "Server.hpp"
#include "ACommand.hpp"
#include "CommandFactory.hpp"

// #define USE_FULL_CLIENT
// #define USE_TESTER
// #ifdef USE_TESTER
// #define USE_FULL_CLIENT
// #endif

Server::~Server()
{
	if (this->_server_socket != -1)
		close(this->_server_socket);
	if (this->_epfd != -1)
		close(this->_epfd);
	for (clientsType::iterator it = this->clients.begin(); it != this->clients.end(); ++it)
	{
		close(it->first);
		delete it->second;
	}
	for (channelsType::iterator it = this->channels.begin(); it != this->channels.end(); ++it)
	{
		delete it->second;
	}
}

Server::Server(int port, std::string password) : _port(port), _password(password), _server_socket(-1), _epfd(-1)
{
	g_sig = 0;
	signal_init();
}

/**
 * @brief Initialize the server socket.
 * This function creates, binds, and listens on a server socket for incoming client connections.
 * It supports both IPv4 and IPv6 addresses.
 * getaddrinfo is used to obtain the address information,
 * it tries each address in a list until it finds one that works.
 *
 * @param[in] port port number to bind the server socket to
 * @return int file descriptor of the server socket on success, -1 on failure
 */
int Server::init_socket(int port)
{
	struct addrinfo hints;
	struct addrinfo *result, *rp;

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;	 /* Allow IPv4 or IPv6 */
	hints.ai_socktype = SOCK_STREAM; /* Stream socket */
	hints.ai_flags = AI_PASSIVE;	 /* For wildcard IP address */
	hints.ai_protocol = 0;			 /* Any protocol */
	hints.ai_canonname = NULL;
	hints.ai_addr = NULL;
	hints.ai_next = NULL;

	{
		int getaddrinfo_ret;
		std::stringstream ss;

		ss << port;
		getaddrinfo_ret = getaddrinfo(NULL, ss.str().c_str(), &hints, &result);
		if (getaddrinfo_ret != 0)
		{
			ss.str("");
			ss << "getaddrinfo: " << gai_strerror(getaddrinfo_ret);
			Debug::print(ERROR, ss.str());
			return -1;
		}
	}
	for (rp = result; rp != NULL; rp = rp->ai_next)
	{
#ifdef SOCK_NONBLOCK
		this->_server_socket = socket(rp->ai_family, rp->ai_socktype | SOCK_NONBLOCK, rp->ai_protocol);
#else
		this->_server_socket = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
#endif
		if (this->_server_socket == -1)
		{
			perror("socket");
			if (rp->ai_next != NULL)
				Debug::print(WARNING, "Could not create socket, trying next...");
			else
				Debug::print(ERROR, "Could not create socket, no more options left.");
			continue;
		}

		int opt = 1;
		if (setsockopt(this->_server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
		{
			perror("setsockopt");
			close(this->_server_socket);
			freeaddrinfo(result);
			return -1;
		}

		if (bind(this->_server_socket, rp->ai_addr, rp->ai_addrlen) == 0)
			break; /* Success */
		perror("bind");
		close(this->_server_socket);
		if (rp->ai_next != NULL)
			Debug::print(WARNING, "Could not bind socket, trying next...");
		else
			Debug::print(ERROR, "Could not bind socket, no more options left.");
	}

	if (rp == NULL) /* No address succeeded */
	{
		freeaddrinfo(result);
		Debug::print(ERROR, "Couldn't find a valid address\n");
		return (-1);
	}
	freeaddrinfo(result); /* No longer needed */
#ifndef SOCK_NONBLOCK
	if (make_fd_nonblocking(this->_server_socket) < 0)
	{
		perror("make_fd_nonblocking");
		close(this->_server_socket);
		return -1;
	}
#endif
	if (listen(this->_server_socket, SOMAXCONN) < 0)
	{
		perror("listen");
		close(this->_server_socket);
		return -1;
	}
	return this->_server_socket;
}

/**
 * @brief Enable EPOLLOUT event for the given file descriptor in epoll instance.
 * This allows the server to be notified when the file descriptor is ready for writing.
 * It is useful to use when the data send to a client couln't be sent in one go
 * and we need to wait for the socket to be writable again.
 *
 * @param[in,out] fd file descriptor of the client socket
 */
void Server::enable_epollout(int fd)
{
	epoll_event ev;
	ev.events = EPOLLIN | EPOLLOUT | EPOLLRDHUP;
	ev.data.fd = fd;
	epoll_ctl(_epfd, EPOLL_CTL_MOD, fd, &ev);
}

// When we wrote in client fd, we don't want epoll_wait to be triggered to write again,
// we switch off the flag EPOLLOUT
/**
 * @brief Disable EPOLLOUT event for the given file descriptor in epoll instance.
 * This prevents the server from being notified when the file descriptor is ready for writing,
 * as it is always ready if there is no data to send.
 *
 * @param[in,out] fd file descriptor of the client socket
 */
void Server::disable_epollout(int fd)
{
	epoll_event ev;
	ev.events = EPOLLIN | EPOLLRDHUP;
	ev.data.fd = fd;
	epoll_ctl(_epfd, EPOLL_CTL_MOD, fd, &ev);
}

/**
 * @brief Make a file descriptor non-blocking.
 *
 * @param[in,out] fd file descriptor to modify
 * @return int 0 on success, -1 on failure
 */
int Server::make_fd_nonblocking(int fd)
{
	if (fcntl(fd, F_SETFL, O_NONBLOCK) == -1)
		return -1;
	return 0;
}

/**
 * @brief Initialize epoll event for a new client file descriptor.
 *
 * @param[in,out] client_fd file descriptor of the new client socket
 * @return int 0 on success, 1 on failure
 */
int Server::init_epoll_event(int client_fd)
{
	epoll_event cev;
	std::memset(&cev, 0, sizeof(cev));
	cev.events = EPOLLIN | EPOLLRDHUP;
	cev.data.fd = client_fd;
	if (epoll_ctl(this->_epfd, EPOLL_CTL_ADD, client_fd, &cev) < 0)
	{
		perror("epoll_ctl add client");
		close(client_fd);
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

/**
 * @brief Initialize a new Client struct and add it to the server's client map.
 *
 * @param[in] client_fd file descriptor of the new client socket
 * @param[in] ip_str IP address of the new client as a string
 * @param[in] port Port number of the new client
 */
void Server::initClient(int client_fd, const std::string &ip_str, uint16_t port)
{
	// since we added a client in our epoll, we need a struct to represent it on our server
	Client *c = new Client(client_fd, ip_str, port);
	this->clients.insert(std::make_pair(client_fd, c));
	std::stringstream ss;
	ss << "New client: " << client_fd;
	Debug::print(DEBUG, ss.str());
}

/**
 * @brief Accept and initialize new client connections.
 * This function continuously accepts new client connections
 * on the server socket until there are no more pending connections.
 */
void Server::new_client()
{
	while (true)
	{
		sockaddr_in client_addr;
		socklen_t client_len = sizeof(client_addr);
		int client_fd = accept(_server_socket, reinterpret_cast<sockaddr *>(&client_addr), &client_len);
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
		if (make_fd_nonblocking(client_fd) < 0)
		{
			perror("make_fd_nonblocking client");
			close(client_fd);
			continue;
		}
		if (init_epoll_event(client_fd))
			continue;
		char ip_str[INET_ADDRSTRLEN];
		uint16_t port = ntohs(client_addr.sin_port);
		inet_ntop(AF_INET, &client_addr.sin_addr, ip_str, sizeof(ip_str));
		initClient(client_fd, ip_str, port);
		// this->clients[client_fd].printClientSocketInfo();
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
	removeClientFromAllChannels(this->clients[fd]);
	epoll_ctl(this->_epfd, EPOLL_CTL_DEL, fd, NULL);
	close(fd);
	delete this->clients[fd];
	this->clients.erase(fd);
	// this->clients[fd]->printClientInfo();
}

void Server::removeClient(Client *client)
{
	if (!client)
		return;
	removeClient(client->_fd);
}

std::string &Server::getPassword()
{
	return this->_password;
}

/**
 * @brief Read data from a client file descriptor.
 * This function attempts to read data from the specified client file descriptor.
 *
 * @param[in,out] fd file descriptor of the client socket
 * @return int 1 on successful read, 0 on client disconnection, -1 on error
 */
int Server::read_client_fd(int fd)
{
	char buf[4096];

	ssize_t r = recv(fd, buf, sizeof(buf), MSG_DONTWAIT);

	if (r >= 512)
	{
		// si on veut traiter les 512 premiers octets il faut read ici
		this->reply(this->clients[fd], "Buffer limit reached, only 512 bytes including \\r\\n allowed");
		std::stringstream ss;
		ss << "Message " << r << " bytes long from " << fd << " ignored";
		Debug::print(INFO, ss.str());
		this->clients[fd]->rbuf.clear();
		return 0; // 1 si on veut lire et traiter les 512 premiers octets
	}

	if (r > 0)
	{
		this->clients[fd]->rbuf.append(buf, &buf[r]);
		// std::stringstream ss;
		// ss << "Received " << r << " bytes from fd " << fd << " [" << this->clients[fd]->rbuf << "]";
		// Debug::print(DEBUG, ss.str());
		return 1;
	}
	else if (r == 0)
	{
		client_quited(fd);
		return 0;
	}
	else
	{
		perror("recv");
		client_quited(fd);
		return -1;
	}
}

// FIXME: ? a mettre en bas de PASS USER et NICK uniquement
/**
 * @brief Check if a client has completed authentication.
 * If the client has provided the correct password, nickname, and username,
 * the server will welcome the new client and mark them as registered.
 *
 * @param[in,out] fd file descriptor of the client socket
 */
void Server::is_authentification_complete(int fd)
{
	if (clients.find(fd) != clients.end() &&
		!clients[fd]->isRegistered() &&
		clients[fd]->isPasswordCorrect() == true &&
		clients[fd]->getNickname() != "" &&
		clients[fd]->getUsername() != "")
	{
		Client *client = this->clients[fd];
		std::stringstream ss;
		ss << client->getNickname() << "!~" << client->getUsername() << "@" << client->getIp();
		client->setHost(ss.str());
		ss.str("");
		this->reply(client, RPL_WELCOME(client->getNickname(), client->getHost()));
		client->setRegistered();
		ss << clients[fd]->getHost() << " successfully connected";
		Debug::print(DEBUG, ss.str());
		// client.printClientIRCInfo();
	}
}

/**
 * @brief Interpret and process messages from a client.
 * This function extracts complete messages from the client's read buffer,
 * parses them into commands, and executes the corresponding actions.
 *
 * @param[in] fd file descriptor of the client socket
 */
void Server::interpret_msg(int fd)
{
	if (this->clients.find(fd) == this->clients.end())
		return;
	size_t pos;
	while ((pos = this->clients[fd]->rbuf.find("\r\n")) != std::string::npos)
	{
		std::string line = this->clients[fd]->rbuf.substr(0, pos);
		this->clients[fd]->rbuf.erase(0, pos + 2);
		if (line.empty())
			continue;
		ACommand *cmd = CommandFactory::findAndCreateCommand(line);
		if (cmd) // valid command found
		{
			cmd->execute(this->clients[fd], *this);
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
		if (this->clients.find(fd) == this->clients.end())
			return;
	}
	// this->clients[fd]->last_ping = std::time(NULL);
	this->is_authentification_complete(fd);
}

/**
 * @brief Handle epoll events, dispatching them to the appropriate handlers.
 * If it's a new connection, it calls new_client().
 * If it's an existing client, it checks for READ, WRITE, HUP,
 * and ERR events and handles them accordingly.
 *
 * @param[in] n number of events returned by epoll_wait
 * @param[in] events event array containing the events to handle
 */
void Server::handle_events(int n, epoll_event events[MAX_EVENTS])
{
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
				if (!this->reply(this->clients[fd], "")) // empty message to trigger write of wbuf != true)
					continue;
			}
			// EPOLLIN : There is data to read in the fd associated
			if (evs & EPOLLIN)
			{
				int result = this->read_client_fd(fd);
				if (result == 1)
					interpret_msg(fd);
			}
		}
	}
}

/**
 * @brief Run the IRC server main loop.
 * Start the server, initialize the socket and epoll instance,
 * and enter the main event loop to handle client connections and messages.
 *
 * @return int 0 on success (signal interrupt is a normal stop), 1 on failure
 */
int Server::RunServer()
{
	this->_server_socket = init_socket(this->_port);
	if (this->_server_socket < 0)
		return EXIT_FAILURE;
	std::stringstream ss;
	ss << "Listening on port: " << this->_port;
	Debug::print(INFO, ss.str());

	this->_epfd = epoll_create(MAX_EVENTS);
	if (this->_epfd < 0)
	{
		perror("epoll_create");
		close(this->_server_socket);
		return (EXIT_FAILURE);
	}
	// doc
	epoll_event ev;
	ev.events = EPOLLIN | EPOLLRDHUP; 
	ev.data.fd = this->_server_socket;
	if (epoll_ctl(this->_epfd, EPOLL_CTL_ADD, this->_server_socket, &ev) < 0)
	{
		perror("epoll_ctl add server");
		close(this->_server_socket);
		close(this->_epfd);
		return EXIT_FAILURE;
	}

	epoll_event events[MAX_EVENTS];
	while (g_sig == 0)
	{
		int n = epoll_wait(this->_epfd, events, MAX_EVENTS, 100); // timeout 100ms -> Max event ??
		if (n < 0)
		{
			if (errno == EINTR) 
			{
				Debug::print(WARNING, "epoll_wait interrupted by signal, closing...");
				break;
			}
			perror("epoll_wait");
			break;
		}
		handle_events(n, events);
		deleteUnusedChannels();
// #ifdef USE_FULL_CLIENT
		// this->check_clients_ping();		 // si on n'a pas eu de signe d'activite depuis trop longtemps
		// this->remove_inactive_clients(); // remove inactive localUsers after a unanswered ping
// #endif
	}
	close(this->_server_socket);
	this->_server_socket = -1;
	close(this->_epfd);
	this->_epfd = -1;
	return EXIT_SUCCESS;
}
