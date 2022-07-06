#include "lvk/app.hpp"

#include <cstdlib>
#include <iostream>
#include <stdexcept>

int main()
{
	lvk::app app{};

	try
	{
		app.run();
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << '\n';
	}

	return 0;
}