#ifndef SERVER_HPP
#define SERVER_HPP

#include <vector>
#include <string>
#include <map>
#include <ctime> 

struct Client {
    int fd;
    std::string rbuf; 
    std::string wbuf; 
    std::time_t last_ping;
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

    Server(std::string password, int port);
    ~Server();
    // Server(const Server& other);
    // Server& operator=(const Server& other);
};

#endif 
