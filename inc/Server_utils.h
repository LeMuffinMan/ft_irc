#ifndef SERVER_UTILS_HPP
#define SERVER_UTILS_HPP

#include <map>

int make_nonblocking(int client_fd);
int init_epoll(int server_fd);
std::string format_time();

#endif // SERVER_UTILS_HPP
