#include "table.h"

#include <iostream>
#include <stdexcept>
#include <tuple>

#include <yaml-cpp/yaml.h>

void process_table(const YAML::Node& n)
{
	table_t t {n};
	t.dump();
	if (!t.is_valid())
	{
		return;
	}

	for (const auto& m : t.available_moves())
	{
		std::cout << std::endl
				  << "Making move \"" << m << "\" : ";
		table_t nt {t};

		bool last {false};
		side_t winer;
		std::tie(last, winer) = nt.make_move(m);

		if (last)
		{
			std::cout << "Last move, winner is " << winer << std::endl;
		}
		else
		{
			std::cout << "Continue turn." << std::endl;
		}
		nt.dump();
	}
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

			process_table(ts);

			std::cout << std::string(40, '=') << std::endl;
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
