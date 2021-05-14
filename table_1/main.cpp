#include <iostream>
#include <stdexcept>

#include "table.h"

#include <yaml-cpp/yaml.h>

void process_table(table_t t)
{
	t.dump();

	if (!t.is_valid()) {
		std::cout << "Table IS NOT VALID: skipped." << std::endl;
		return;
	}
	std::cout << "Table valid." << std::endl;
}

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

			process_table(table_t {ts});

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
