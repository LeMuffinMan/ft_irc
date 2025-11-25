#ifndef SERVER_HPP
#define SERVER_HPP

#include <vector>
#include <string>
#include <map>
#include <ctime> 
#include <sys/epoll.h>
#include "NetworkState.hpp"

const int MAX_EVENTS = 64; //Faire une taille dynamique (au fil de l'eau -> vecteur)
                           //Interet des bornes ? deinfe / global

class Server {
  private:
    int _port;
    std::vector<int> _localUsers_fd;
    const std::string _password;
    std::map<int, LocalUser> _localUsers;
    int _server_socket;
    int _epfd;
    NetworkState *_networkState;

    Server(); // On ne veut pas de serveur sans mdp ni sans port
  public:
    // int getPort() const;
    // int checkPassword();
    void RunServer();
    int init_network(NetworkState &networkState);
    int init_socket(int port);
    int read_client_fd(int fd);
    void handle_events(int n, epoll_event events[MAX_EVENTS]);
    void send_welcome(int fd, int epfd);
    void remove_inactive_localUsers(int epfd);
    void check_localUsers_ping(int epfd);
    void enable_epollout(int fd);
    int write_client_fd(int fd);
    void new_client(int server_fd);
    void client_quited(int fd); // leaved plutot que quited
    void send_welcome(int fd);
    void remove_inactive_localUsers();
    void check_localUsers_ping();

    Server(int port, std::string password);
    ~Server();
    // Server(const Server& other);
    // Server& operator=(const Server& other);
};

#endif 
