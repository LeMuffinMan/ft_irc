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

void pong_command(std::string line, int fd, std::map<int, LocalUser>& localUser)
{
	size_t colon = line.find(':');
	if (colon != std::string::npos) {
		std::string ts_str = line.substr(colon + 1);
		std::istringstream iss(ts_str);
		long sent_time = localUser[fd].last_ping;
		if (iss >> sent_time) {
			std::time_t now = std::time(NULL);
			long latency = now - sent_time;
			std::cout << format_time() << " [" << line << "] from client " << fd << " received (" << latency << " secs)" << std::endl;
			localUser[fd].timeout = LONG_MAX;
		} else {
			std::cerr << "Invalid timestamp in PONG: " << ts_str << std::endl;
			//un pirate ? on veut surement le deco
		}
	}
}

void pass_command(std::string line, int fd, std::map<int, LocalUser>& localUser, std::string password)
{
	size_t colon = line.find(' ');
	if (colon != std::string::npos) {
		std::string pw_str = line.substr(colon + 1);
		std::istringstream iss(pw_str);
		std::cout << format_time() << " [" << line << "] from client " << fd << std::endl;
		if (pw_str == password)
			localUser[fd].client->_password_correct = true;
		else
		{
			std::cout << "NUM ERROR: Incorrect password" << std::endl;
		}
	}
}


void nick_command(std::string line, int fd, std::map<int, LocalUser>& localUser)
{
	size_t colon = line.find(' ');
	if (colon != std::string::npos) {
		std::string nn_str = line.substr(colon + 1);
		std::istringstream iss(nn_str);
		std::cout << format_time() << " [" << line << "] from client " << fd << std::endl;
		localUser[fd].client->setNickname(nn_str);
	}
}

void user_command(std::string line, int fd, std::map<int, LocalUser>& localUser)
{
	size_t colon = line.find(':');
	if (colon != std::string::npos) {
		std::string us_str = line.substr(colon + 1);
		std::istringstream iss(us_str);
		std::cout << format_time() << " [" << line << "] from client " << fd << std::endl;
		localUser[fd].client->setUsername(us_str);
	}
}
