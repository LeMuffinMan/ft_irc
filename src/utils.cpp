#include <utils.hpp>
#include <sys/stat.h>
#include <unistd.h>

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
