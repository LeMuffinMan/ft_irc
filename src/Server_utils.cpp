#include <cstdlib>
#include <fcntl.h> // fcntl, O_NONBLOCK
#include <iomanip>
#include <sstream>
#include <iostream>
#include "Server.hpp" // /!\ pour localUser

int init_epoll(int server_fd)
{
	int epfd = epoll_create(MAX_EVENTS);
	if (epfd < 0)
	{
		perror("epoll_create");
		close(server_fd);
		exit(EXIT_FAILURE);
	}
	return epfd;
}
// fcntl et NON_BLOCK uniquemet pour MAC OS ? si oui faire un gros if, et gerer autrement la lecture / ecriture : sinon ca marche
int make_nonblocking(int fd)
{
	if (fcntl(fd, F_SETFL, O_NONBLOCK) == -1)
		return -1;
	return 0;
}

std::string format_time(void)
{
	std::time_t now = time(NULL);
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

std::string format_date(void)
{
    std::time_t now = std::time(NULL);
    const int timezone_offset = 3600;
    now += timezone_offset;
    
    std::tm* timeinfo = std::gmtime(&now);
    
    const char* days[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
    const char* months[] = {"January", "February", "March", "April", "May", "June", 
                           "July", "August", "September", "October", "November", "December"};
    
    std::ostringstream oss;
    oss << days[timeinfo->tm_wday] << " " 
        << months[timeinfo->tm_mon] << " " 
        << timeinfo->tm_mday << " " 
        << (timeinfo->tm_year + 1900);
    
    return oss.str();
}
