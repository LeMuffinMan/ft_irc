#ifndef SERVER_HPP
#define SERVER_HPP

#include <vector>
#include <string>
#include <map>
#include <ctime> 
#include <sys/epoll.h>


const int MAX_EVENTS = 64; //Faire une taille dynamique (au fil de l'eau -> vecteur)
                           //Interet des bornes ? deinfe / global
struct Client {
    int fd;
    std::string rbuf; 
    std::string wbuf; 
    bool password;
    std::string nickname;
    std::string user;
    std::time_t last_ping;
    std::time_t timeout;
};

class Server {
  private:
    int _port;
    std::vector<int> _clients_fd;
    const std::string _password;
    std::map<int, Client> _Clients;
    int _server_socket;
    int _epfd;

    Server(); // On ne veut pas de serveur sans mdp ni sans port
  public:
    int getPort() const;
    int checkPassword();
    void RunServer();
    int init_socket(int port);
    int read_client_fd(int fd, int epfd, std::map<int, Client>& clients);
    void handle_events(int server_fd, int epfd, std::map<int, Client>& clients, int n, epoll_event events[MAX_EVENTS]);
    void send_welcome(int fd, int epfd, std::map<int, Client>& clients);
    void remove_inactive_clients(int epfd, std::map<int, Client>& clients);
    void check_clients_ping(int epfd, std::map<int, Client>& clients);

    Server(std::string password, int port);
    ~Server();
    // Server(const Server& other);
    // Server& operator=(const Server& other);
};

#endif 
