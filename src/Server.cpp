#include <sys/types.h>
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

Server::Server(std::string password, std::string port) :  _port(port), _password(password) {}

void Server::Run()
{
  //creer un 1er socket, qui permettra d'accepter des connnexions de clients, pour chaque accept, un nouveau socket
  int init_socket = socket(AF_INET, SOCK_STREAM, 0);
  if(init_socket < 0)
  {
      perror("socket()");
      exit(errno);
  }
  std::cout << "init_socket = " << init_socket << std::endl;

  sockaddr_in sin;

  //htonl htons : big / little endian : ordre des bits et compatibilite
  sin.sin_addr.s_addr = htonl(INADDR_ANY); 
  sin.sin_family = AF_INET;
  sin.sin_port = htons(6667);

  //Une fois le socket initialise, on veut le binder a une interface : ca permettra d'ecouter, puis d'accepter une connexion d'un client 
  if(bind (init_socket, (sockaddr *) &sin, sizeof sin) < 0)
  {
      perror("bind()");
      exit(errno);
  }

  //mtn que le init_socket est binde a l'interface, on peut listen <=> attendre une demande de connexion d'un client
  if(listen(init_socket, 5) < 0)
  {
      perror("listen()");
      exit(errno);
  }

  sockaddr_in csin;
  int csock;
  int sinsize = sizeof csin;


  //ici on va avoir besoin de poll / epoll : une fois debloque, on peut accepter un client en recuperant le socket associe  
  csock = accept(init_socket, (sockaddr *)&csin, (socklen_t *)&sinsize);

  if(csock < 0)
  {
      perror("accept()");
      exit(errno);
  }

  close(init_socket);
  // while (1)
  // {

    //Attendre la connexion d'un client poll / epoll 
      //COnnexion ? Commande ?
        //connexion
          //attribuer et stocker un fd 
          //papoter pour dire que c'est bon 
        //commande 
          //controler user chanell ... 
          //parser la commande
          //executer la commande  
        //Timeout epoll ?
  // }
}

Server::~Server() {}
