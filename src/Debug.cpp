#include "Debug.hpp"
#include "utils.hpp"

std::ostream *Debug::stream = &std::cout;
std::ostream *Debug::errStream = &std::cerr;
bool Debug::useColors = false;

Debug::Debug()
{
}

Debug::~Debug()
{
}

Debug::Debug(const Debug &f)
{
	(void)f;
}

Debug &Debug::operator=(const Debug &other)
{
	(void)other;
	return (*this);
}

/**
 * @brief Change the stream
 *
 * @param[in,out] newStream stream that will replaced the old
 * @return std::ostream* old stream
 */
std::ostream *Debug::changeStream(std::ostream &newStream)
{
	std::ostream *temp;
	stream->flush();
	temp = stream;
	stream = &newStream;
	return (temp);
}

/**
 * @brief Change the errStream
 *
 * @param[in,out] newStream stream that will replaced the old
 * @return std::ostream* old errStream
 */
std::ostream *Debug::changeErrStream(std::ostream &newStream)
{
	std::ostream *temp;
	errStream->flush();
	temp = errStream;
	errStream = &newStream;
	return (temp);
}

void Debug::changeUseColor(bool val)
{
	useColors = val;
}

/*
Example will print one on stream,
two and three will be written in file.txt
and four will be written on first stream

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
