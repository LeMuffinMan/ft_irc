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

struct Client {
    int fd;
    std::string rbuf;  //to read msg
    std::string wbuf; //to send back
};


int make_nonblocking(int fd) {
  
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1)
      return -1;

    //on ajoute O_NONBLOCK aux flags existants 
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
      return -1;
    return 0;
}

int init_socket(const std::string &port_str) {
    //flags ?
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0) {
        perror("socket");
        return -1;
    }

  // ????
    int opt = 1;
    if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        close(listen_fd);
        return -1;
    }

    sockaddr_in sin;
    ::memset(&sin, 0, sizeof(sin));

    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = htonl(INADDR_ANY);
    int port = atoi(port_str.c_str());
  //gerer l'erreur ici 
    sin.sin_port = htons(static_cast<uint16_t>(port));

  //doc 
    if (bind(listen_fd, (sockaddr*)&sin, sizeof(sin)) < 0) {
        perror("bind");
        close(listen_fd);
        return -1;
    }
//flags ?
    if (listen(listen_fd, SOMAXCONN) < 0) {
        perror("listen");
        close(listen_fd);
        return -1;
    }

    if (make_nonblocking(listen_fd) < 0) {
        perror("make_nonblocking");
        close(listen_fd);
        return -1;
    }
    return listen_fd;
}

void Server::RunServer() {
    int server_fd = init_socket(this->_port);
    std::cout << "Now listening on port: " << this->_port << std::endl;
    if (server_fd < 0) exit(EXIT_FAILURE);
    int epfd = epoll_create(64);
    if (epfd < 0) {
      perror("epoll_create");
      close(server_fd);
      exit(EXIT_FAILURE);
    }

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

    //Voir comment on stock les clients, un vec de classes ?
    std::map<int, Client> clients;
    //faire un define 
    const int MAX_EVENTS = 64;
    //init a tab of events 
    epoll_event events[MAX_EVENTS];

    while (true) {
        int n = epoll_wait(epfd, events, MAX_EVENTS, -1);
        if (n < 0) {
            //a tester pour garder 
            if (errno == EINTR)
              continue; // signal interrompt -> relancer
            perror("epoll_wait");
            break;
        }

    //for each event received during epoll_wait
        for (int i = 0; i < n; ++i) {
            //What fd is it ? 
              //case 1 : server_fd = new client
              //case 2 : msg to parse
            int fd = events[i].data.fd;
            uint32_t evs = events[i].events;

            if (fd == server_fd) {
                while (true) {
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
                    clients.insert(std::make_pair(client_fd, c));                    std::cout << "New client: " << client_fd << std::endl;
                }
            } else {
                //error handling 
                  //EPOLLHUP EPOLLERR ??
                if (evs & (EPOLLHUP | EPOLLERR)) {
                    std::cerr << "EPOLLERR/HUP on fd " << fd << std::endl;
                    epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
                    close(fd);
                    //class clients a faire ?
                    clients.erase(fd);
                    //on continue sans quit ?
                    continue;
                }
                if (evs & EPOLLRDHUP) {
                    // client fermé côté distant
                    std::cout << "Remote closed: " << fd << std::endl;
                    epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
                    close(fd);
                    clients.erase(fd);
                    continue;
                }

                //its an event to read 
                if (evs & EPOLLIN) {
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
                            break;
                        } else {
                            //Quels sont les autres cas couverts ?
                            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                                break; // On a fini de parser les events ?
                            } else {
                                perror("recv");
                                closed = true;
                                break;
                            }
                        }
                    }
                    //close bool ?
                    if (closed) {
                        epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
                        close(fd);
                        clients.erase(fd);
                        continue;
                    }
                }
            }
        }
    }

    // cleanup
    close(server_fd);
    close(epfd);
}

Server::~Server() {}

Server::Server(std::string port, std::string password) :  _port(port), _password(password) {}

