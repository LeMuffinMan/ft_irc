#ifndef DEBUG_TPP
#define DEBUG_TPP

#include "Debug.hpp"
#include "utils.hpp"

/**
 * @brief Print a debug message with a specific level
 *
 * @param[in] level debug level
 * @param[in] message message to print
 */
template <typename T>
void Debug::print(debug_level level, const T &message)
{
	switch (level)
	{
	case NOTHING:
		return;
	case INFO:
		*stream << format_time() << " [INFO] ";
		break;
	case DEBUG:
		*stream << format_time() << " [DEBUG] ";
		break;
	case WARNING:
		*stream << format_time() << " [WARNING] ";
		break;
	case REPLY:
		*stream << format_time() << " [REPLY] ";
		break;
	case ERR_REPLY:
		if (useColors)
			*errStream << format_time() << " [" << RED << "ERR_REPLY" << RESET << "] ";
		else
			*errStream << format_time() << " [ERR_REPLY] ";
		break;
	case ERROR:
		if (useColors)
			*errStream << format_time() << " [" << RED << "ERROR" << RESET << "] ";
		else
			*errStream << format_time() << " [ERROR] ";
		break;
	default:
		break;
	}
	if (level == ERROR)
		*errStream << message << std::endl;
	else
		*stream << message << std::endl;
}

/**
 * @brief Print a debug message with a specific level on standard output or err
 *
 * @param[in] level debug level
 * @param[in] message message to print
 */
template <typename T>
void Debug::printSTD(debug_level level, const T &message)
{
	switch (level)
	{
	case NOTHING:
		return;
	case INFO:
		std::cout << format_time() << " [INFO] ";
		break;
	case DEBUG:
		std::cout << format_time() << " [DEBUG] ";
		break;
	case WARNING:
		std::cout << format_time() << " [WARNING] ";
		break;
	case REPLY:
		std::cout << format_time() << " [REPLY] ";
		break;
	case ERR_REPLY:
		if (useColors)
			std::cerr << format_time() << " [" << RED << "ERR_REPLY" << RESET << "] ";
		else
			std::cerr << format_time() << " [ERR_REPLY] ";
		break;
	case ERROR:
		if (useColors)
			std::cerr << format_time() << " [" << RED << "ERROR" << RESET << "] ";
		else
			std::cerr << format_time() << " [ERROR] ";
		break;
	default:
		break;
	}
	if (level == ERROR)
		std::cerr << message << std::endl;
	else
		std::cout << message << std::endl;
}

#endif // DEBUG_TPP
