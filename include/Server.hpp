#ifndef SERVER_HPP
#define SERVER_HPP

#include <vector>
#include <string>

class Server {
  private:
    // int port;
    std::string _port;
    std::vector<int> clients_fd;
    const std::string _password;

    Server();
  public:
    int getPort() const;
    int checkPassword();
    void Run();

    Server(std::string password, std::string port);
    ~Server();
    Server(const Server& other);
    Server& operator=(const Server& other);
};

//1 attendre et recevoir des requettes des clients
    //socket 
    //buffer-> dependnant des clients
    
//2 parsing la requete
    //formattage : pour discriminer connect / disconnect / command normales 

//3 Executer la requete 
    //connection
        //Timeout 
    //


#endif 
