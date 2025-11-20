#include <cstdlib>
#include <sys/epoll.h>
#include <cstring>
#include <string>
#include <unistd.h>     // close, read, write, etc.
#include <fcntl.h>      // fcntl, O_NONBLOCK
#include <sys/socket.h> // socket, setsockopt, bind, listen
#include <netinet/in.h> // sockaddr_in, INADDR_ANY, htons
#include <cstdio>

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

int make_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
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
    ::memset(&sin, 0, sizeof(sin));

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

