#include "utils.hpp"
#include <sys/stat.h>
#include <unistd.h>
#include <iomanip>
#include <sstream>
#include <ctime>

static bool is_terminal(int fd);

bool is_stdout_on_tty()
{
	return is_terminal(1);
}

bool is_stderr_on_tty()
{
	return is_terminal(2);
}

bool is_stdin_on_tty()
{
	return is_terminal(0);
}

/**
 * @brief Check if a given path is a terminal
 *
 * @param[in] fd file descriptor to check
 * @return true true if the path is a terminal
 * @return false false if the path is not a terminal
 */
bool is_terminal(int fd)
{
	struct stat st;
	if (fstat(fd, &st) == -1)
		return false;
	return S_ISCHR(st.st_mode);
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

// TODO: strftime from ctime could be used instead
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
