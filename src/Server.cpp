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

//define ou global ?
const int MAX_EVENTS = 64; //Faire une taille dynamique (au fil de l'eau -> vecteur)
						   //Interet des bornes ? deinfe / global


void new_client(int server_fd, int epfd, std::map<int, LocalUser>& clients) {
	while (true) {
		//while we can register a new client we do so
		int client_fd = accept(server_fd, NULL, NULL);
		if (client_fd < 0) {
			//avoid blocking epoll : in case there is no more client to accept
			if (errno == EAGAIN || errno == EWOULDBLOCK)
				break; 
			perror("accept");
			break;
		}
		make_nonblocking(client_fd);

		epoll_event cev;
		std::memset(&cev, 0, sizeof(cev));
		//we set en epoll_event, triggered in case of data to read or
		//if client close its writing end (fragmented msgs)
		cev.events = EPOLLIN | EPOLLRDHUP;
		//we bind this new client event struct, with the client fd
		cev.data.fd = client_fd;
		//adding our new fd, and the event struct to our epoll, with the events we just set
		if (epoll_ctl(epfd, EPOLL_CTL_ADD, client_fd, &cev) < 0) {
			perror("epoll_ctl add client");
			close(client_fd);
			continue;
		}
		//Deviendra une classe ? Une struct suffit ici
		LocalUser c;
		c.fd = client_fd;
		c.rbuf = "";
		c.wbuf = "";
		
		//si clients.insert a foire, gerer la collision ?

		//use make pair to use the fd as key and the client struct as data
		clients.insert(std::make_pair(client_fd, c));  //si les buf changent de taille undefined behavior ? si oui, stocker des pointeurs (alloc) 
		std::cout << "New client: " << client_fd << std::endl;
	}
}

void client_quited(int fd, int epfd, std::map<int, LocalUser>& clients) // leaved plutot que quited
{
	std::cout << "Remote closed: " << fd << std::endl;
	//we remove the leaving client fd, and the event struct associated
	epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
	close(fd);
	//we delete in our map the leaving client
	clients.erase(fd);
}

//To fix
//return 0 : all done
//return 1 : there is still data to read
//return -1 : error to handle
//
//Revoir le recv pour les \r\n et faire les tests avec nc -C 127.0.0.1 6667 PUIS des CTRL+D 
//
int read_client_fd(int fd, int epfd, std::map<int, LocalUser>& clients)
{
	bool closed = false;
	while (true) {
		//read in the read buf 
		char buf[4096];
		ssize_t r = recv(fd, buf, sizeof(buf), 0); // TESTER : deux clients, un qui envoie la bible, l'autre qui envoie un petit text : pas de blocage !
		if (r > 0) {
			clients[fd].rbuf.append(buf, buf + r); //&buf[r]plus safe sur l'espacement mémoire, unicodes ça ne marchera pas
			//on cherche a extraire le message, delimite par un \r\n ??
			size_t pos;
			//\r ET \n ou \r OU \n ?
			//A casse avec le refacto !
			while ((pos = clients[fd].rbuf.find("\r\n")) != std::string::npos) { //make non blocking va pas marcher ici ? et si on peut pas avoir GETFLG encore moins ?
				std::string line = clients[fd].rbuf.substr(0, pos);
				clients[fd].rbuf.erase(0, pos + 2);
				std::cout << "msg from " << fd << ": [" << line << "]\n";
				//HERE, we want to parse th msg, and then answer using wbuf
			}
		} else if (r == 0) {
			// client closed cleanly
			closed = true;
			close(fd);
			clients.erase(fd);
			return 1;
		} else {
			// autres cas a couvrir ? EINT ?
			if (errno == EAGAIN || errno == EWOULDBLOCK) {
				return 1; 
			} else {
				perror("recv");
				closed = true;
				close(fd);
				clients.erase(fd);
				return 1;
			}
		}
	}
	//non reachable 
	if (closed) {
		epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
		close(fd);
		clients.erase(fd);
		return 0;
	}
}

void handle_events(int server_fd, int epfd, std::map<int, LocalUser> clients, int n, epoll_event events[MAX_EVENTS])
{
	//for each event received during epoll_wait
	for (int i = 0; i < n; ++i) {
		int fd = events[i].data.fd;
		uint32_t evs = events[i].events;

		if (fd == server_fd) {
			new_client(server_fd, epfd, clients);    
		} else {
			// HUP : fd closed by client : the socket is dead
			// ERR : Error on fd
			if (evs & (EPOLLHUP | EPOLLERR)) { // in case of EPOLLHUP / EPOLLRDHUP : we clean our map, but is there any other possibility of client leaving without saying ?
				std::cerr << "EPOLLERR/HUP on fd " << fd << std::endl;
				client_quited(fd, epfd, clients);
				continue;
			}
			//RDHUP :  client closed fd, the socket is still alive  
			if (evs & EPOLLRDHUP) {
				std::cout << "EPOLLRDHUP on fd " << fd << std::endl;
				client_quited(fd, epfd, clients);
				continue;
			}
			//EPOLLIN : There is data to read in the fd associated 
			if (evs & EPOLLIN) {
				if (read_client_fd(fd, epfd, clients))
					break;
				else
					continue;
			}
		}
	}
}

void Server::RunServer() {
	int server_fd = init_socket(this->_port);
	if (server_fd < 0)
	{
		std::cout << "Failed to listen on port" << this->_port << std::endl;
		exit(EXIT_FAILURE);
	}
	std::cout << "Now listening on port: " << this->_port << std::endl;

	int epfd = init_epoll(server_fd);
	//doc 
	epoll_event ev;
	ev.events = EPOLLIN | EPOLLRDHUP; // RDHUP pour détecter fermeture distante
	ev.data.fd = server_fd;
	if (epoll_ctl(epfd, EPOLL_CTL_ADD, server_fd, &ev) < 0) {
		perror("epoll_ctl add server");
		close(server_fd);
		close(epfd);
		exit(EXIT_FAILURE);
	}

	//hash map pour associer chaque client a son fd : acceder a chaque client en utilisant son fd comme cle 
	std::map<int, LocalUser> clients;
	epoll_event events[MAX_EVENTS];

	while (true) {
		//we check for events from our clients fd registered
		int n = epoll_wait(epfd, events, MAX_EVENTS, -1);
		if (n < 0) {
			// if (errno == EINTR)
			//   continue; // signal interrompt -> relancer
			perror("epoll_wait");
			break;
		}
		handle_events(server_fd, epfd, clients, n, events);
	}
	close(server_fd);
	close(epfd);
}

Server::~Server() {}

Server::Server(std::string port, std::string password) :  _port(port), _password(password) {}


void Server::reply(Client* client, std::string message)
{
	std::cout << "Reply to " << client->nickname << ": " << message << std::endl;
}

void Server::replyChannel(Channel& channel, std::string message)
{
	for (std::set<Client*>::iterator it = channel.getClients().begin(); it != channel.getClients().end(); ++it)
	{
		reply(*it, message);
	}
}

void Server::replyChannelOnlyOP(Channel& channel, std::string message)
{
	for (std::set<Client*>::iterator it = channel.getOperators().begin(); it != channel.getOperators().end(); ++it)
	{
		reply(*it, message);
	}
}

void Server::broadcast(NetworkState& network, std::string message)
{
	(void)network;
	std::cout << "Broadcast message: " << message << std::endl;
}

void Server::noticeServers(NetworkState& network, std::string message)
{
	(void)network;
	std::cout << "Notice servers: " << message << std::endl;
}
