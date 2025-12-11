#ifndef DEBUG_HPP
#define DEBUG_HPP

#include <iostream>

enum debug_level
{
	NOTHING,
	INFO,
	REPLY,
	ERR_REPLY,
	DEBUG,
	WARNING,
	ERROR
};

#define RESET "\033[0m"
#define RESET_COLOR RESET
#define RED "\033[31m"

// #ifndef DEBUG_LEVEL
// #define DEBUG_LEVEL NOTHING
// #endif

class Debug
{
	//= Variables =//
private:
	static std::ostream *stream;	//!< Output stream for debug messages
	static std::ostream *errStream; //!< Output stream for error messages

public:
	static bool useColors; //!< Flag to enable/disable colored output

	//= Methods =//
private:
	// Orthodox Canonical Form : Constructors / Destructors / Operators
	Debug();
	~Debug();
	Debug(const Debug &f);
	Debug &operator=(const Debug &other);

public:
	static std::ostream *changeStream(std::ostream &newStream);
	static std::ostream *changeErrStream(std::ostream &newStream);
	static void changeUseColor(bool val);

	//= Templates =//
	template <typename T>
	static void print(debug_level level, const T &content);
	template <typename T>
	static void printSTD(debug_level level, const T &content);
};

#include "Debug.tpp"

/*
Example will print 'one' on ErrStream,
'two' and 'three' will be written in file.txt
and 'four' will be written on first stream

void example(void)
{
	Debug::print(ERROR, "one");
	std::ofstream s;
	s.open("file.txt");
	std::ostream *t = Debug::changeErrStream(s);
	Debug::print(ERROR, "two");
	Debug::useColors = true;
	Debug::print(ERROR, "three");
	Debug::changeErrStream(*t);
	Debug::print(ERROR, "four");
	return ;
}
*/

#endif // DEBUG_HPP
