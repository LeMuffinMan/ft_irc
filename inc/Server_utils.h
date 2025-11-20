#ifndef SERVER_UTILS_H
#define SERVER_UTILS_H

int make_nonblocking(int client_fd);
int init_socket(const std::string& port);
int init_epoll(int server_fd);

#endif
