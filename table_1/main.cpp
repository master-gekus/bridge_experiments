#include <iostream>
#include <fstream>
#include <stdexcept>

#include "table.h"

int main(int argc, char** argv)
{
	if (2 > argc)
	{
		std::cout << "Argument needed." << std::endl;
		return 1;
	}

	std::ifstream df {argv[1]};
	if (!df.is_open())
	{
		std::cout << "Failed to open \"" << argv[1] << "\"." << std::endl;
		return 1;
	}

	try
	{
		while (!df.eof())
		{
			cards c;
			df >> c;
			std::cout << c << std::endl;
		}
	}
	catch (const std::exception& e)
	{
		std::cout << "Exception: " << e.what() << std::endl;
		return 1;
	}
	return 0;
}
