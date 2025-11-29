#include <cstdlib>
#include <map>
#include <sys/epoll.h>
#include <cstring>
#include <string>
#include <unistd.h>     // close, read, write, etc.
#include <fcntl.h>      // fcntl, O_NONBLOCK
#include <sys/socket.h> // socket, setsockopt, bind, listen
#include <netinet/in.h> // sockaddr_in, INADDR_ANY, htons
#include <cstdio>
#include <ctime>
#include <iomanip>
#include <sstream>
#include "Server.hpp" // /!\ pour localUser
#include <iostream>
#include <climits>

int init_epoll(int server_fd)
{
	int epfd = epoll_create(MAX_EVENTS);
	if (epfd < 0) {
		perror("epoll_create");
		close(server_fd);
		exit(EXIT_FAILURE);
	}
	return epfd;
}

int make_nonblocking(int fd) { //fcntl et NON_BLOCK uniquemet pour MAC OS ? si oui faire un gros if, et gerer autrement la lecture / ecriture : sinon ca marche
	if (fcntl(fd, F_SETFL, O_NONBLOCK) == -1)
		return -1;
	return 0;
}
