#include <cstdlib>
#include <sys/epoll.h>
#include <cstring>
#include <string>
#include <unistd.h>     // close, read, write, etc.
#include <fcntl.h>      // fcntl, O_NONBLOCK
#include <sys/socket.h> // socket, setsockopt, bind, listen
#include <netinet/in.h> // sockaddr_in, INADDR_ANY, htons
#include <cstdio>

//TOUTES ces fonctions sont supposees etre des methodes 

int init_epoll(int server_fd)
{
    int epfd = epoll_create(64);
    if (epfd < 0) {
      perror("epoll_create");
      close(server_fd);
      exit(EXIT_FAILURE);
    }
    return epfd;
}

int make_nonblocking(int fd) { //fcntl et NON_BLOCK uniquemet pour MAC OS ? si oui faire un gros if, et gerer autrement la lecture / ecriture : sinon ca marche
    int flags = fcntl(fd, F_GETFL, 0); //On en n'a pas besoin ?  
    if (flags == -1)
      return -1;

    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
      return -1;
    return 0;
}

//To documentate
int init_socket(const std::string &port_str) {
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0) {
        perror("socket");
        return -1;
    }

    int opt = 1;
    if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        close(listen_fd);
        return -1;
    }

    sockaddr_in sin;
    std::memset(&sin, 0, sizeof(sin)); //avec ou sans std ?

    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = htonl(INADDR_ANY);
    int port = atoi(port_str.c_str());
    if (port < 0) // ou > a max port ? 
    {
        perror("atoi");
        close(listen_fd);
        return -1;
    }
    sin.sin_port = htons(static_cast<uint16_t>(port));

    if (bind(listen_fd, (sockaddr*)&sin, sizeof(sin)) < 0) {
        perror("bind");
        close(listen_fd);
        return -1;
    }
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

