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

Server::~Server() {}

Server::Server(int port, std::string password) :  _port(port), _password(password) {}

int Server::init_network(NetworkState &networkState)
{
    this->_networkState = &networkState;
    return 0;
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

int Server::write_client_fd(int fd)
{
    std::string &wbuf = _localUsers[fd].wbuf;

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
            _localUsers.erase(fd);
            return -1;
        }
    }

    epoll_event ev;
    ev.events = EPOLLIN | EPOLLRDHUP;  // remove EPOLLOUT
    ev.data.fd = fd;
    epoll_ctl(this->_epfd, EPOLL_CTL_MOD, fd, &ev);

    return 0;
}

void Server::new_client(int server_fd) {
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
        LocalUser c;
        c.fd = client_fd;
        c.rbuf = "";  
        c.wbuf = "";
        c.last_ping = std::time(NULL);
        c.timeout = LONG_MAX; // 1min pour repondre PONG
        Client* client = new Client(client_fd);
        c.client = client;
        client->setLocalClient(&c);
        client->setUsername("RandomUser");
        // si localUsers.insert a foire, gerer la collision ?
        // use make pair to use the fd as key and the client struct as data
        this->_localUsers.insert(std::make_pair(client_fd, c));   
        this->_networkState->addClient("", client); // adding client to network state
        std::cout << format_time() << " New client: " << client_fd << std::endl;
    }
}

void Server::client_quited(int fd) // leaved plutot que quited
{
    std::cout << "Remote closed: " << fd << std::endl;
    //we remove the leaving client fd, and the event struct associated
    epoll_ctl(this->_epfd, EPOLL_CTL_DEL, fd, NULL);
    close(fd);
    //we delete in our map the leaving client
    this->_localUsers.erase(fd);
}

void Server::send_welcome(int fd)
{
    std::stringstream ss;
    ss << this->_localUsers[fd].client->getUsername() << " aka " << this->_localUsers[fd].client->getNickname() << " successfully registered" << "\r\n";
    this->_localUsers[fd].wbuf += ss.str();
    this->enable_epollout(fd);
    this->write_client_fd(fd);
}

void Server::remove_inactive_localUsers()
{
    for (std::map<int, LocalUser>::iterator it = this->_localUsers.begin(); it != this->_localUsers.end(); ++it)
    {
        int fd = it->first;
        LocalUser& localuser = it->second;
        
        std::time_t now = std::time(NULL);
        if (now > localuser.timeout) // 1 min
        {
            std::stringstream ss;
            ss << this->_localUsers[fd].client->getUsername() << " aka " << this->_localUsers[fd].client->getNickname() << " timed out" << "\r\n";
            this->_localUsers[fd].wbuf += ss.str();
            this->enable_epollout(fd);
            this->write_client_fd(fd);
            close(fd);
            this->_localUsers.erase(fd);
        }
    }
}

void Server::check_localUsers_ping()
{
    for (std::map<int, LocalUser>::iterator it = this->_localUsers.begin(); it != this->_localUsers.end(); ++it)
    {
        int fd = it->first;
        LocalUser& client = it->second;

        std::time_t now = std::time(NULL);
        if (now - client.timeout > 5) // 5 sec
        {
            std::stringstream ss;
            std::time_t now = std::time(NULL);
            ss << "PING :" << now << "\r\n";
            client.wbuf += ss.str();
            this->_localUsers[fd].timeout = now + 5000; // 5 sec de timeout 
            this->enable_epollout(fd);
            client.last_ping = now;
            std::cout << format_time() << " [PING :" << now << "] sent to client " << fd << std::endl; 
        }
    }
}

int Server::read_client_fd(int fd)
{
    char buf[4096];
    
    // MSG_DONTWAIT : rend non bloquant et suffisant ?
    ssize_t r = recv(fd, buf, sizeof(buf), MSG_DONTWAIT);
    
    if (r > 0) {
        this->_localUsers[fd].rbuf.append(buf, buf + r); // &buf[r]
        // std::cout << "Received " << r << " bytes from fd " << fd << std::endl;
        
        // 512 max
        //  - on execute ce qu'on peut
        //  - on jette le reste : on clean, et on relit pas un truc qu'on veut pas lire


        size_t pos;
        while ((pos = this->_localUsers[fd].rbuf.find("\r\n")) != std::string::npos) {
            std::string line = this->_localUsers[fd].rbuf.substr(0, pos);
            this->_localUsers[fd].rbuf.erase(0, pos + 2);
            
            // TODO : will be moved to another fct
            std::cout << "Processing: [" << line << "]" << std::endl;
            if (line.rfind("PONG", 0) == 0) {
                pong_command(line, fd, this->_localUsers);
            }
            else if (line.rfind("PASS", 0) == 0) {
                pass_command(line, fd, this->_localUsers, this->_password);
            }
            else if (line.rfind("NICK", 0) == 0) {
                nick_command(line, fd, this->_localUsers);
            }
            else if (line.rfind("USER", 0) == 0) {
                user_command(line, fd, this->_localUsers);    
            }
            else {
                std::cout << "msg from " << fd << ": [" << line << "]" << std::endl;
            }
            
            this->_localUsers[fd].last_ping = std::time(NULL);
            
            if (!this->_localUsers[fd].client->registered && 
                this->_localUsers[fd].client->password_correct == true && 
                this->_localUsers[fd].client->getNickname() != "" && 
                this->_localUsers[fd].client->getUsername() != "") {
                
                this->send_welcome(fd);
                this->_localUsers[fd].client->registered = true;
                std::cout << this->_localUsers[fd].client->getUsername() << " aka " << this->_localUsers[fd].client->getNickname() << " successfully connected" << std::endl;
            }
        }
        
        return 1;
        
    } else if (r == 0) {
        std::cout << "Client " << fd << " disconnected" << std::endl;
        epoll_ctl(this->_epfd, EPOLL_CTL_DEL, fd, NULL);
        close(fd);
        this->_localUsers.erase(fd);
        return 0;
    } else {
        //error handling
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return 1;
        } else {
            perror("recv");
            epoll_ctl(this->_epfd, EPOLL_CTL_DEL, fd, NULL);
            close(fd);
            this->_localUsers.erase(fd);
            return 0;
        }
    }
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
                std::cerr << "EPOLLERR/HUP on fd " << fd << std::endl;
                this->client_quited(fd);
                continue;
            }
            //RDHUP :  client closed fd, the socket is still alive  
            if (evs & EPOLLRDHUP) {
                std::cout << "EPOLLRDHUP on fd " << fd << std::endl;
                this->client_quited(fd);
                continue;
            }
            //EPOLLIN : There is data to read in the fd associated 
            if (evs & EPOLLIN) {
                int result = this->read_client_fd(fd);
                // 0 = client disconnected
                if (result == 0) {
                    continue;
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
    std::map<int, LocalUser> localUsers;
    epoll_event events[MAX_EVENTS];

    while (true) {
        //we check for events from our localUsers fd registered
        int n = epoll_wait(this->_epfd, events, MAX_EVENTS, 2000); //timeout 2 sec -> revoir la gestion du timeout : retour de fct / teste evs
        if (n < 0) {
            // if (errno == EINTR)
            //   continue; // signal interrompt -> relancer
            perror("epoll_wait");
            break;
        }
        handle_events(n, events);
        this->check_localUsers_ping(); //si on n'a pas eu de signe d'activite depuis trop longtemps
        this->remove_inactive_localUsers(); // remove inactive localUsers after a unanswered ping
    }
    close(this->_server_socket);
    close(this->_epfd);
}

