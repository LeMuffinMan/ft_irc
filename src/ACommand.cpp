#include "ACommand.hpp"

ACommand::~ACommand()
{
}

void ACommand::print_params() const
{
	std::cout << "Command parameters:" << std::endl;
	for (size_t i = 0; i < _params.size(); ++i)
	{
		std::cout << "Param " << i << ": " << _params[i] << std::endl;
	}
}
