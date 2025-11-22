#include <cstdlib>
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

std::string format_time() {
    std::time_t now = time(NULL);
    //decalage horraire par rapport a utc + 0
    const int timezone_offset = 3600; 
    now += timezone_offset;

    std::time_t seconds_in_day = now % 86400; 
    int hours = static_cast<int>(seconds_in_day / 3600);
    int minutes = static_cast<int>((seconds_in_day % 3600) / 60);
    int seconds = static_cast<int>(seconds_in_day % 60);

    std::ostringstream oss;
    oss << std::setw(2) << std::setfill('0') << hours << ":"
        << std::setw(2) << std::setfill('0') << minutes << ":"
        << std::setw(2) << std::setfill('0') << seconds;

    return oss.str();
}
