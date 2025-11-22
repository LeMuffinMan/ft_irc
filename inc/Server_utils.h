#ifndef SERVER_UTILS_H
#define SERVER_UTILS_H

#include <map>

int make_nonblocking(int client_fd);
int init_epoll(int server_fd);
std::string format_time();

//place_holders
void pass_command(std::string line, int fd, std::map<int, Client>& clients, std::string password);
void nick_command(std::string line, int fd, std::map<int, Client>& clients);
void user_command(std::string line, int fd, std::map<int, Client>& clients);
void pong_command(std::string line, int fd, std::map<int, Client>& clients);

#endif
