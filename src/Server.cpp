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
#include <string.h>
#include <map> 
#include "Server_utils.h"



//define ou global ?
const int MAX_EVENTS = 64;

struct Client {
    int fd;
    std::string rbuf; 
    std::string wbuf; 
};


void new_client(int server_fd, int epfd, std::map<int, Client> clients) {
    while (true) {
        //while we can register a new client we do so
        int client_fd = accept(server_fd, NULL, NULL);
        if (client_fd < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) break; // fini pour l'instant
            perror("accept");
            break;
        }
        make_nonblocking(client_fd);

        epoll_event cev;
        //Doc ???
        cev.events = EPOLLIN | EPOLLRDHUP; // lire + detecter fermeture
        cev.data.fd = client_fd;
        if (epoll_ctl(epfd, EPOLL_CTL_ADD, client_fd, &cev) < 0) {
            perror("epoll_ctl add client");
            close(client_fd);
            continue;
        }
        //init client
        Client c;
        c.fd = client_fd;
        c.rbuf = "";
        c.wbuf = "";
        //use make pair to use the fd as key and the client struct as data
        clients.insert(std::make_pair(client_fd, c));   
        std::cout << "New client: " << client_fd << std::endl;
    }
}

void client_quited(int fd, int epfd, std::map<int, Client> clients)
{
    std::cout << "Remote closed: " << fd << std::endl;
    epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
    close(fd);
    clients.erase(fd);
}

int read_client_fd(int fd, int epfd, std::map<int, Client> clients)
{
    bool closed = false;
    while (true) {
        //read in the read buf 
        char buf[4096];
        ssize_t r = recv(fd, buf, sizeof(buf), 0);
        if (r > 0) {
            clients[fd].rbuf.append(buf, buf + r);
            //on cherche a extraire le message, delimite par un \r\n ??
            size_t pos;
            while ((pos = clients[fd].rbuf.find("\r\n")) != std::string::npos) {
                std::string line = clients[fd].rbuf.substr(0, pos);
                clients[fd].rbuf.erase(0, pos + 2);
                std::cout << "msg from " << fd << ": [" << line << "]\n";
                //HERE, we want to parse th msg, and then answer using wbuf
            }
        } else if (r == 0) {
            // client closed cleanly
            closed = true;
            return 1;
        } else {
            //Quels sont les autres cas couverts ?
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                return 1; // On a fini de parser les events ?
            } else {
                perror("recv");
                closed = true;
                return 1;
            }
        }
    }
    if (closed) {
        epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
        close(fd);
        clients.erase(fd);
        return 0;
    }
}

void handle_events(int server_fd, int epfd, std::map<int, Client> clients, int n, epoll_event events[MAX_EVENTS])
{
    //for each event received during epoll_wait
    for (int i = 0; i < n; ++i) {
        int fd = events[i].data.fd;
        uint32_t evs = events[i].events;

        if (fd == server_fd) {
            new_client(server_fd, epfd, clients);    
        } else {
            //error handling 
                //EPOLLHUP EPOLLERR ??
            if (evs & (EPOLLHUP | EPOLLERR)) {
                std::cerr << "EPOLLERR/HUP on fd " << fd << std::endl;
                epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
                close(fd);
                clients.erase(fd);
                //on continue sans quit ?
                continue;
            }
            if (evs & EPOLLRDHUP) {
                client_quited(fd, epfd, clients);
                continue;
            }
            //its an event to read 
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
    std::cout << "Now listening on port: " << this->_port << std::endl;
    if (server_fd < 0)
        exit(EXIT_FAILURE);

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

    //hash map pour associer chaque client a son fd 
    std::map<int, Client> clients;
    //init a tab of events 
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

