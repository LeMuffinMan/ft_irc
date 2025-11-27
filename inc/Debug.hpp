#ifndef DEBUG_HPP
#define DEBUG_HPP

#include <iostream>

enum debug_level
{
	NOTHING = 0,
	INFO = 1,
	DEBUG = 2,
	ERROR = 3
};

# define RESET "\033[0m"
# define RESET_COLOR RESET
# define RED "\033[31m"

// #ifndef DEBUG_LEVEL
// #define DEBUG_LEVEL NOTHING
// #endif

class Debug
{
//= Variables =//
private:
	static std::ostream* stream;	//!< Output stream for debug messages
	static std::ostream* errStream;	//!< Output stream for error messages

public:
	static bool useColors;			//!< Flag to enable/disable colored output

//= Methods =//
private:
	// Orthodox Canonical Form : Constructors / Destructors / Operators
	Debug();
	~Debug();
	Debug(const Debug &f);
	Debug& operator=(const Debug& other);

public:
	static std::ostream* changeStream(std::ostream &newStream);
	static std::ostream* changeErrStream(std::ostream &newStream);
	static void changeUseColor(bool val);

	//= Templates =//
	template <typename T>
	static void print(debug_level level, const T &content);
	template <typename T>
	static void printSTD(debug_level level, const T &content);
};

#include "Debug.tpp"

#endif // DEBUG_HPP
