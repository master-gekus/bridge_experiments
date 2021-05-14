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

			std::cout << "N : " << cards(ts["North"]) << std::endl;
			std::cout << "E : " << cards(ts["East"]) << std::endl;
			std::cout << "S : " << cards(ts["South"]) << std::endl;
			std::cout << "W : " << cards(ts["West"]) << std::endl;

//			std::cout << "ts.IsScalar(): " << std::boolalpha << ts.IsScalar() << std::endl;


			std::cout << std::string (40, '=') << std::endl;
			std::cout << std::endl;
		}
//		auto yaml{YAML::LoadFile(argv[1])};
//		if (!yaml.IsSequence()) {
//			std::cout << "Not a sequense!: " << std::endl;
//			return 1;
//		}

//		while (!df.eof())
//		{
//			cards c;
//			df >> c;
//			std::cout << c << std::endl;
//		}
	}
	catch (const std::exception& e)
	{
		std::cout << "Exception: " << e.what() << std::endl;
		return 1;
	}
	return 0;
}
