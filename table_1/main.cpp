#include <iostream>
#include <iomanip>
#include <stdexcept>

#include "table.h"

#include <yaml-cpp/yaml.h>

int main(int argc, char** argv)
{
	if (2 > argc)
	{
		std::cout << "Argument needed." << std::endl;
		return 1;
	}

	try
	{
		std::size_t index {0};
		for (const auto& ts : YAML::LoadFile(argv[1]))
		{
			std::cout << std::string(40, '=') << std::endl;
			std::cout << "Table #" << (++index) << std::endl;

			std::cout << "  N:" << std::endl;
			hand {ts["N"]}.dump();

			std::cout << "  E:" << std::endl;
			hand {ts["E"]}.dump();

			std::cout << "  S:" << std::endl;
			hand {ts["S"]}.dump();

			std::cout << "  W:" << std::endl;
			hand {ts["W"]}.dump();

			std::cout << std::string (40, '=') << std::endl;
			std::cout << std::endl;
		}
	}
	catch (const std::exception& e)
	{
		std::cout << "Exception: " << e.what() << std::endl;
		return 1;
	}
	return 0;
}
