#include <sys/types.h>
#include <climits>
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

    if (bind(this->_server_socket, (sockaddr*)&sin, sizeof(sin)) < 0) {
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

void Server::enable_epollout(int fd)
{
    epoll_event ev;
    ev.events = EPOLLIN | EPOLLOUT | EPOLLRDHUP;
    ev.data.fd = fd;
    epoll_ctl(this->_epfd, EPOLL_CTL_MOD, fd, &ev);
}

int Server::write_client_fd(int fd, std::map<int, Client>& clients)
{
    std::string &wbuf = clients[fd].wbuf;

    while (!wbuf.empty()) {
        ssize_t n = send(fd, wbuf.data(), wbuf.size(), 0);

        if (n > 0) {
            wbuf.erase(0, n);
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

    epoll_event ev;
    ev.events = EPOLLIN | EPOLLRDHUP;  // remove EPOLLOUT
    ev.data.fd = fd;
    epoll_ctl(this->_epfd, EPOLL_CTL_MOD, fd, &ev);

    return 0;
}

void Server::new_client(int server_fd, std::map<int, Client>& clients) {
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
        if (epoll_ctl(this->_epfd, EPOLL_CTL_ADD, client_fd, &cev) < 0) {
            perror("epoll_ctl add client");
            close(client_fd);
            continue;
        }
        //Deviendra une classe ? Une struct suffit ici
        //faire une init pour chaque struct objet
        Client c;
        c.fd = client_fd;
        c.rbuf = "";
        c.wbuf = "";
        c.password = false;
        c.nickname = "";
        c.user = "";
        c.last_ping = std::time(NULL);
        c.timeout = LONG_MAX; // 1min pour repondre PONG 
        
        //si clients.insert a foire, gerer la collision ?
        //use make pair to use the fd as key and the client struct as data
        clients.insert(std::make_pair(client_fd, c));   

        std::cout << format_time() << " New client: " << client_fd << std::endl;
    }
}

void Server::client_quited(int fd, std::map<int, Client>& clients) // leaved plutot que quited
{
    std::cout << "Remote closed: " << fd << std::endl;
    //we remove the leaving client fd, and the event struct associated
    epoll_ctl(this->_epfd, EPOLL_CTL_DEL, fd, NULL);
    close(fd);
    //we delete in our map the leaving client
    clients.erase(fd);
}

void Server::send_welcome(int fd, std::map<int, Client>& clients)
{
    std::stringstream ss;
    ss << clients[fd].user << " aka " << clients[fd].nickname << " successfully registered" << "\r\n";
    clients[fd].wbuf += ss.str();
    this->enable_epollout(fd);
    this->write_client_fd(fd, clients);
}

void Server::remove_inactive_clients(std::map<int, Client>& clients)
{
    for (std::map<int, Client>::iterator it = clients.begin(); it != clients.end(); ++it)
    {
        int fd = it->first;
        Client& client = it->second;
        
        std::time_t now = std::time(NULL);
        if (now > client.timeout) // 1 min
        {
            std::stringstream ss;
            ss << clients[fd].user << " aka " << clients[fd].nickname << " timed out" << "\r\n";
            clients[fd].wbuf += ss.str();
            this->enable_epollout(fd);
            this->write_client_fd(fd, clients);
            close(fd);
            clients.erase(fd);
        }
    }
}

void Server::check_clients_ping(std::map<int, Client>& clients)
{
    for (std::map<int, Client>::iterator it = clients.begin(); it != clients.end(); ++it)
    {
        int fd = it->first;
        Client& client = it->second;

        std::time_t now = std::time(NULL);
        if (now - client.last_ping > 5) // 5 sec
        {
            std::stringstream ss;
            std::time_t now = std::time(NULL);
            ss << "PING :" << now << "\r\n";
            client.wbuf += ss.str();
            clients[fd].timeout = now + 5000; // 5 sec de timeout 
            this->enable_epollout(fd);
            client.last_ping = now;
            std::cout << format_time() << " [PING :" << now << "] sent to client " << fd << std::endl; 
        }
    }
}


int Server::read_client_fd(int fd, std::map<int, Client>& clients)
{
    char buf[4096];
    
    // MSG_DONTWAIT : rend non bloquant et suffisant ?
    ssize_t r = recv(fd, buf, sizeof(buf), MSG_DONTWAIT);
    
    if (r > 0) {
        clients[fd].rbuf.append(buf, buf + r); // &buf[r]
        // std::cout << "Received " << r << " bytes from fd " << fd << std::endl;
        
        size_t pos;
        while ((pos = clients[fd].rbuf.find("\r\n")) != std::string::npos) {
            std::string line = clients[fd].rbuf.substr(0, pos);
            clients[fd].rbuf.erase(0, pos + 2);
            
            // std::cout << "Processing: [" << line << "]" << std::endl;
            
            if (line.rfind("PONG", 0) == 0) {
                pong_command(line, fd, clients);
            }
            else if (line.rfind("PASS", 0) == 0) {
                pass_command(line, fd, clients, this->_password);
            }
            else if (line.rfind("NICK", 0) == 0) {
                nick_command(line, fd, clients);
            }
            else if (line.rfind("USER", 0) == 0) {
                user_command(line, fd, clients);    
            }
            else {
                std::cout << "msg from " << fd << ": [" << line << "]" << std::endl;
            }
            
            clients[fd].last_ping = std::time(NULL);
            
            if (!clients[fd].registered && 
                clients[fd].password == true && 
                clients[fd].nickname != "" && 
                clients[fd].user != "") {
                
                this->send_welcome(fd, clients);
                clients[fd].registered = true;
                std::cout << clients[fd].user << " aka " << clients[fd].nickname << " successfully connected" << std::endl;
            }
        }
        
        return 1;
        
    } else if (r == 0) {
        std::cout << "Client " << fd << " disconnected" << std::endl;
        epoll_ctl(this->_epfd, EPOLL_CTL_DEL, fd, NULL);
        close(fd);
        clients.erase(fd);
        return 0;
    } else {
        //error handling
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return 1;
        } else {
            perror("recv");
            epoll_ctl(this->_epfd, EPOLL_CTL_DEL, fd, NULL);
            close(fd);
            clients.erase(fd);
            return 0;
        }
    }
}

void Server::handle_events(std::map<int, Client>& clients, int n, epoll_event events[MAX_EVENTS])
{
    //for each event received during epoll_wait
    for (int i = 0; i < n; ++i) {
        int fd = events[i].data.fd;
        uint32_t evs = events[i].events;

        if (fd == this->_server_socket) {
            this->new_client(this->_server_socket, clients);    
        } else {
            // HUP : fd closed by client : the socket is dead
            // ERR : Error on fd
            if (evs & (EPOLLHUP | EPOLLERR)) { // in case of EPOLLHUP / EPOLLRDHUP : we clean our map, but is there any other possibility of client leaving without saying ?
                std::cerr << "EPOLLERR/HUP on fd " << fd << std::endl;
                this->client_quited(fd, clients);
                continue;
            }
            //RDHUP :  client closed fd, the socket is still alive  
            if (evs & EPOLLRDHUP) {
                std::cout << "EPOLLRDHUP on fd " << fd << std::endl;
                this->client_quited(fd, clients);
                continue;
            }
            //EPOLLIN : There is data to read in the fd associated 
            if (evs & EPOLLIN) {
                int result = this->read_client_fd(fd, clients);
                // 0 = client disconnected
                if (result == 0) {
                    continue;
                }
            }

            //EPOLLOUT : We set that flag when we write in a client buffer, we need to send it
            if (evs & EPOLLOUT) {
                if (this->write_client_fd(fd, clients) < 0)
                    continue;
            }
        }
    }
}

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
    std::map<int, Client> clients;
    epoll_event events[MAX_EVENTS];

    while (true) {
        //we check for events from our clients fd registered
        int n = epoll_wait(this->_epfd, events, MAX_EVENTS, 2000); //timeout 2 sec -> revoir la gestion du timeout : retour de fct / teste evs
        if (n < 0) {
            // if (errno == EINTR)
            //   continue; // signal interrompt -> relancer
            perror("epoll_wait");
            break;
        }
        handle_events(clients, n, events);
        this->check_clients_ping(clients); //si on n'a pas eu de signe d'activite depuis trop longtemps
        this->remove_inactive_clients(clients); // remove inactive clients after a unanswered ping
    }
    close(this->_server_socket);
    close(this->_epfd);
}

Server::~Server() {}

Server::Server(std::string password, int port) :  _port(port), _password(password) {}

