#include <sys/types.h>
#include <iomanip>
#include <sstream>
#include <ctime>
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
#include "Server.hpp"
#include "ACommand.hpp"
#include "CommandFactory.hpp"

Server::~Server()
{
	// delete this->_networkState;
	for (std::map<int, Client>::iterator it = this->clients.begin(); it != this->clients.end(); ++it)
	{
		close(it->first);
		// delete it->second.client;
	}
	for (std::map<std::string, Channel*>::iterator it = this->channels.begin(); it != this->channels.end(); ++it)
	{
		delete it->second;
	}
}

Server::Server(int port, std::string password) :  _port(port) 
{
	g_sig = 0;
	_password = password;
	signal_init();
}

std::string format_time() {
	std::time_t now = time(NULL);
	//decalage horraire par rapport a utc + 0
	const int timezone_offset = 3600; 
	now += timezone_offset;

	std::time_t seconds_in_day = now % 86400; 
	int hours = static_cast<int>(seconds_in_day / 3600);
	int minutes = static_cast<int>((seconds_in_day % 3600) / 60);
	int seconds = static_cast<int>(seconds_in_day % 60);

	std::ostringstream oss;
	oss << std::setw(2) << std::setfill('0') << hours << ":"
		<< std::setw(2) << std::setfill('0') << minutes << ":"
		<< std::setw(2) << std::setfill('0') << seconds;
	return oss.str();
}

//To documentate
int Server::init_socket(int port) {
	this->_server_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (this->_server_socket < 0) {
		perror("socket");
		return -1;
	}

	int opt = 1;
	if (setsockopt(this->_server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
		perror("setsockopt");
		close(this->_server_socket);
		return -1;
	}

	sockaddr_in sin;
	::memset(&sin, 0, sizeof(sin));

	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = htonl(INADDR_ANY);

	sin.sin_port = htons(static_cast<uint16_t>(port));

	if (bind(this->_server_socket, reinterpret_cast<const sockaddr*>(&sin), sizeof(sin)) < 0) {
		perror("bind");
		close(this->_server_socket);
		return -1;
	}
	if (listen(this->_server_socket, SOMAXCONN) < 0) {
		perror("listen");
		close(this->_server_socket);
		return -1;
	}

	if (make_nonblocking(this->_server_socket) < 0) {
		perror("make_nonblocking");
		close(this->_server_socket);
		return -1;
	}
	return this->_server_socket;
}

//When we want to write in a socket, using this function will trigger epoll_wait
//and lead us to write_client_fd
void Server::enable_epollout(int fd)
{
	epoll_event ev;
	ev.events = EPOLLIN | EPOLLOUT | EPOLLRDHUP;
	ev.data.fd = fd;
	epoll_ctl(_epfd, EPOLL_CTL_MOD, fd, &ev);
}

//When we wrote in client fd, we don't want epoll_wait to be triggered to write again,
//we switch off the flag EPOLLOUT
void Server::disable_epollout(int fd)
{
	epoll_event ev;
	ev.events = EPOLLIN | EPOLLRDHUP;
	ev.data.fd = fd;
	epoll_ctl(_epfd, EPOLL_CTL_MOD, fd, &ev);
}

//to doc
int Server::write_client_fd(int fd)
{
	std::string &wbuf = clients[fd].wbuf;

	while (!wbuf.empty()) {
		ssize_t n = send(fd, wbuf.data(), wbuf.size(), 0);

		if (n > 0) {
			wbuf.erase(0, static_cast<size_t>(n));
		}
		//socket buffer full 
		else if (n == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
			return 0;
		}
		else {
			// error or client disconnected
			std::cerr << "send error on fd " << fd << "\n";
			epoll_ctl(this->_epfd, EPOLL_CTL_DEL, fd, NULL);
			close(fd);
			clients.erase(fd);
			return -1;
		}
	}
	this->disable_epollout(fd);
	return 0;
}

int Server::init_epoll_event(int client_fd)
{
	//Necessaire ?
	make_nonblocking(client_fd);

	//each client registered in epoll_ctl must have an event struct associated
	epoll_event cev;
	std::memset(&cev, 0, sizeof(cev));
	//theses flags define what we want to trigger epoll_wait :
	//In case of data to read or
	//if client closed its writing end (fragmented msgs)
	cev.events = EPOLLIN | EPOLLRDHUP;
	//we bind this new client event struct, with the client fd
	cev.data.fd = client_fd;
	//adding our new fd, and the event struct to our epoll, with the events we just set
	if (epoll_ctl(this->_epfd, EPOLL_CTL_ADD, client_fd, &cev) < 0) {
		perror("epoll_ctl add client");
		close(client_fd);
		return 1;
	}
	return 0;

}

void Server::init_localuser(int client_fd)
{
	//since we added a client in our epoll, we need a struct to represent it on our server
	//LocalUser's Attributes contains the pipes and tools, Client contains its server infos
	Client c;
	c.fd = client_fd;
	//we want to kick incactives clients
	c.last_ping = std::time(NULL);
	c.connection_time = std::time(NULL);
	c.timeout = -1;
	// the client object contains 
    this->clients.insert(std::make_pair(client_fd, c));   
	std::cout << format_time() << " New client: " << client_fd << std::endl;
}

//for each call to accept with our server fd, if there is a client to register, it will returns a > 0 fd
//We register all clients available with the true loop, and break only in case of error, or when there is no client to register
void Server::new_client(int server_fd) {
	while (true) {
		int client_fd = accept(server_fd, NULL, NULL);
		if (client_fd < 0) {
			if (errno == EAGAIN || errno == EWOULDBLOCK)
				break;
			if (errno == EMFILE || errno == ENFILE) {
				Debug::print(ERROR, "TOO MANY FILE DESCRIPTORS!");
			}
			perror("accept");
			break;
		}
		if (init_epoll_event(client_fd))
			continue;
		init_localuser(client_fd);
	}
}

void Server::client_quited(int fd)
{
    std::stringstream ss;
    ss << "Remote closed: " << fd;
    Debug::print(INFO, ss.str());
	// std::cout << "Remote closed: " << fd << std::endl;
	//we must stop track this fd with our epoll
	epoll_ctl(this->_epfd, EPOLL_CTL_DEL, fd, NULL);
	close(fd);
	//we delete in our map the leaving client
	this->clients.erase(fd);
}

//A remplacer avec un send reply
void Server::send_welcome(int fd)
{
	std::stringstream ss;
	ss << this->clients[fd].getUsername() << " aka " << this->clients[fd].getNickname() << " successfully registered" << "\r\n";
	this->clients[fd].wbuf += ss.str();
	this->enable_epollout(fd);
	this->write_client_fd(fd);
}

std::string Server::get_command(std::string line) 
{
	size_t pos = line.find(' ');
	if (pos == std::string::npos) {
		return line;
	}
	return line.substr(0, pos);
}

std::vector<std::string> Server::get_params(std::string line)
{
	std::vector<std::string> params;
	std::string last_param;
	
	size_t pos = line.find(' ');
	
	if (pos == std::string::npos) {
		return params;
	}

	std::string remaining = line.substr(pos + 1);
	std::string after_colon;
	bool has_colon = remaining.find(':') != std::string::npos;
	if (has_colon) {
		size_t colon_pos = remaining.find(':');
		after_colon = remaining.substr(colon_pos + 1);
		remaining = remaining.substr(0, colon_pos);
	}
	std::stringstream ss(remaining);
	std::string param;
	char del = ' ';
	while (getline(ss, param, del))
		params.push_back(param);
	if (has_colon)
		params.push_back(after_colon);
	return params;
}

void Server::removeLocalUser(int fd) {
    close(fd);
	epoll_ctl(this->getEpfd(), EPOLL_CTL_DEL, fd, NULL);
    this->clients.erase(fd);
}

int Server::getEpfd() const {
    return _epfd;
}

std::string& Server::getPassword()
{
	return this->_password;
}

ACommand* Server::parse_command(std::string line)
{
        std::cout << "Processing: [" << line << "]" << std::endl;

        std::string cmd = this->get_command(line);
        if (cmd.empty())
            return NULL;
        std::vector<std::string> params = this->get_params(line);
        return CommandFactory::createCommand(cmd, params);
}

//returns 1 to send a buff in parsing
//returns 0 in case of client disconnection
//returns -1 in case of error
int Server::read_client_fd(int fd)
{
	char buf[4096];
	
	// MSG_DONTWAIT : rend non bloquant et suffisant ?
	ssize_t r = recv(fd, buf, sizeof(buf), MSG_DONTWAIT);
	
	if (this->clients[fd].rbuf.size() >= 512) {
		//On peut envoyer un message "not allowed msg > 512"

		//si on veut traiter les 512 premiers octets il faut read ici 

		std::cerr << "Buffer limit reached for fd " << fd << ", cleaning buffer" << std::endl;
		this->clients[fd].rbuf.clear();
		return 0; //1 si on veut lire et traiter les 512 premiers octets
	}

	if (r > 0) {
		this->clients[fd].rbuf.append(buf, buf + r); // &buf[r]
		// std::cout << "Received " << r << " bytes from fd " << fd << std::endl;
		return 1;
		
	} else if (r == 0) {
		std::cout << "Client " << fd << " disconnected" << std::endl;
		epoll_ctl(this->_epfd, EPOLL_CTL_DEL, fd, NULL);
		close(fd);
		this->clients.erase(fd);
		return 0;
	} else {
		//error handling : ici il faut que le parsing attende !
		//Ces flags sont importants si on utilise EPOLET, sans, c'est peut etre superflu
		// if (errno == EAGAIN || errno == EWOULDBLOCK) {
		//     return 2; // 1 -> 2
		// } else {
			perror("recv");
			epoll_ctl(this->_epfd, EPOLL_CTL_DEL, fd, NULL);
			close(fd);
			this->clients.erase(fd);
			return -1;
		// }
	}
}

//a mettre en bas de PASS USER et NICK uniquement
void Server::is_authentification_complete(int fd)
{
	if (!this->clients[fd].isRegistered() && 
		this->clients[fd].isPasswordCorrect() == true && 
		this->clients[fd].getNickname() != "" && 
		this->clients[fd].getUsername() != "") {
		
		this->send_welcome(fd);
		std::cout << this->clients[fd].getUsername() << " aka " << this->clients[fd].getNickname() << " successfully connected" << std::endl;
		this->clients[fd].setRegistered(true);
	}
}

void Server::interpret_msg(int fd)
{
	size_t pos;
	while ((pos = this->clients[fd].rbuf.find("\r\n")) != std::string::npos) {
		std::string line = this->clients[fd].rbuf.substr(0, pos);
		this->clients[fd].rbuf.erase(0, pos + 2);
		ACommand* cmd = this->parse_command(line);      
		//try catch ?
		if (cmd)
		{
			cmd->execute(&this->clients[fd], *this);
			delete cmd;
			// this->clients[fd].client->printClientInfo();
		}
	}
	this->clients[fd].last_ping = std::time(NULL);
	this->is_authentification_complete(fd);
}

void Server::handle_events(int n, epoll_event events[MAX_EVENTS])
{
	//for each event received during epoll_wait
	for (int i = 0; i < n; ++i) {
		int fd = events[i].data.fd;
		uint32_t evs = events[i].events;

        if (fd == this->_server_socket) {
            this->new_client(this->_server_socket);    
        } else {
            // HUP : fd closed by client : the socket is dead
            // ERR : Error on fd
            if (evs & (EPOLLHUP | EPOLLERR)) { // in case of EPOLLHUP / EPOLLRDHUP : we clean our map, but is there any other possibility of client leaving without saying ?
				std::stringstream ss;
				ss << "EPOLLERR/HUP on fd " << fd;
				Debug::print(ERROR, ss.str());
                this->client_quited(fd);
                continue;
            }
            //RDHUP :  client closed fd, the socket is still alive  
            if (evs & EPOLLRDHUP) {
                std::cout << "EPOLLRDHUP on fd " << fd << std::endl;
                this->client_quited(fd);
                continue;
            }
          	//EPOLLOUT : We set that flag when we write in a client buffer, we need to send it
			// if (evs & EPOLLOUT) {
			// 	std::cout << "L'erreur est bien ici !" << fd << std::endl;
			// 	Server::reply(this->_localUsers[fd].client, "");
			// }
			//EPOLLIN : There is data to read in the fd associated 
			if (evs & EPOLLIN) {
				int result = this->read_client_fd(fd);
				// 0 = client disconnected
				if (result == 0) {
					continue;
				} else if (result == 1 ) {
					interpret_msg(fd);
				} else if (result < 0) {
					//error handling
				}
			}
			//EPOLLOUT : We set that flag when we write in a client buffer, we need to send it
			if (evs & EPOLLOUT) {
				if (this->write_client_fd(fd) < 0)
					continue;
			}
		}
	}
}

//revoir ici max event et la logique
void Server::RunServer() {

	this->_server_socket = init_socket(this->_port);
	if (this->_server_socket < 0)
		exit(EXIT_FAILURE);
	std::cout << format_time() << " Listening on port: " << this->_port << std::endl;

	this->_epfd = init_epoll(this->_server_socket);
	//doc 
	epoll_event ev;
	ev.events = EPOLLIN | EPOLLRDHUP; // RDHUP pour détecter fermeture distante
									  //EPOLLET permet de rendre les sockets non bloquants ?;
	ev.data.fd = this->_server_socket;
	if (epoll_ctl(this->_epfd, EPOLL_CTL_ADD, this->_server_socket, &ev) < 0) {
		perror("epoll_ctl add server");
		close(this->_server_socket);
		close(this->_epfd);
		exit(EXIT_FAILURE);
	}

	//hash map pour associer chaque client a son fd : acceder a chaque client en utilisant son fd comme cle 
	epoll_event events[MAX_EVENTS];

	while (g_sig == 0) {
		//we check for events from our localUsers fd registered
		int n = epoll_wait(this->_epfd, events, MAX_EVENTS, 100); //timeout 100ms 
		if (n < 0) {
			// if (errno == EINTR)
			//   continue; // signal interrompt -> relancer
			perror("epoll_wait");
			break;
		}
		handle_events(n, events);
		// this->check_clients_ping(); //si on n'a pas eu de signe d'activite depuis trop longtemps
		// this->remove_inactive_clients(); // remove inactive clients after a unanswered ping
	}
	close(this->_server_socket);
	close(this->_epfd);
}

//on mettrait pas le timestamp dans le debug aussi ?
bool Server::reply(Client* client, std::string message)
{
	if (!client)
	{
		Debug::print(ERROR, "No client given");
		return false;
	}
	std::string &wbuf = client->wbuf;
	if (!message.empty())
		wbuf.append(message).append("\r\n");
	while (!wbuf.empty())
	{
		ssize_t n = send(client->fd, wbuf.c_str(), wbuf.size(), 0);

		if (n > 0) {
			Debug::print(INFO, "Reply to " + client->getNickname() + ": " + wbuf.substr(0, static_cast<size_t>(n - 1)));
			wbuf.erase(0, static_cast<size_t>(n));
		}
		//socket buffer full 
		else if (n == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
			Debug::print(INFO, "The message couldn't be send in one try, retrying next time");
			enable_epollout(client->fd);
			return 0;
		}
		else {
			// FIXME: Need to call client_kicked here !
			// error or client disconnected
			std::cerr << "[ERROR] Send error on fd " << client->fd << "\n";
			epoll_ctl(_epfd, EPOLL_CTL_DEL, client->fd, NULL);
			close(client->fd);
			clients.erase(client->fd);
			return false;
		}
	}
	return true;
}

bool Server::replyChannel(Channel* channel, std::string message)
{
	if (!channel)
	{
		Debug::print(ERROR, "No channel given");
		return false;
	}
	bool ret = true;
	for (std::set<Client *>::iterator it = channel->getClients().begin(); it != channel->getClients().end(); ++it)
	{
		if (!reply(*it, message))
		{
			ret = false;
			Debug::print(ERROR, "The following message couldn't be properly send: " + message);
		}
	}
	return ret;
}

bool Server::replyChannelOnlyOP(Channel* channel, std::string message)
{
	if (!channel)
	{
		Debug::print(ERROR, "No channel given");
		return false;
	}
	bool ret = true;
	for (std::set<Client*>::iterator it = channel->getOperators().begin(); it != channel->getOperators().end(); ++it)
	{
		if (!reply(*it, message))
		{
			ret = false;
			Debug::print(ERROR, "The following message couldn't be properly send: " + message);
		}
	}
	return ret;
}

//a retester
void Server::remove_inactive_clients()
{
	std::time_t now = std::time(NULL);

    for (std::map<int, Client>::iterator it = this->clients.begin();
         it != this->clients.end(); )
    {
        int fd = it->first;
        Client& client = it->second;
        now = std::time(NULL);
        if ((client.timeout > 0 && now > client.timeout) || (!client.isRegistered() && client.connection_time + 7 < now))
        {
            std::stringstream ss;
            if (client.isRegistered())
            {
                // ss << localuser.client->getUsername()
                // << " aka " << localuser.client->getNickname()
                // << " timed out\r\n";
                this->reply(&client, "timed out");

            }
            else {
                // ss << "Disconnected: timed out" << std::endl;
                this->reply(&client, "timed out");
            }

			client.wbuf += ss.str();
			this->enable_epollout(fd);
			this->write_client_fd(fd);

            close(fd);
	        epoll_ctl(this->_epfd, EPOLL_CTL_DEL, fd, NULL);
            this->clients.erase(it++);   
        }
        else
        {
            ++it; 
        }
    }
}


//a retester
void Server::check_clients_ping()
{
	for (std::map<int, Client>::iterator it = this->clients.begin(); 
		 it != this->clients.end(); ++it)
	{
		int fd = it->first;
		Client& client = it->second;

		std::time_t now = std::time(NULL);
		
		if (now - client.last_ping > PING_INTERVAL) 
		{
			std::stringstream ss;
			ss << "PING :" << now << "\r\n";
			client.wbuf += ss.str();
			client.timeout = now + 3;
			this->enable_epollout(fd);
			client.last_ping = now;
			ss.clear();
			ss << "[PING :" << now << "] sent to client " << it->first;
			Debug::print(DEBUG, ss.str());
		}
	}
}

