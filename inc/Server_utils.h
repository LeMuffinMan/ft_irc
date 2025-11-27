#ifndef SERVER_UTILS_HPP
#define SERVER_UTILS_HPP

#include <map>

int make_nonblocking(int client_fd);
int init_epoll(int server_fd);
std::string format_time();

//place_holders
void pass_command(std::string line, int fd, std::map<int, LocalUser>& localUser, std::string password);
void nick_command(std::string line, int fd, std::map<int, LocalUser>& localUser);
void user_command(std::string line, int fd, std::map<int, LocalUser>& localUser);
void pong_command(std::string line, int fd, std::map<int, LocalUser>& localUser);

#endif  // SERVER_UTILS_HPP
