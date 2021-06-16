#include <cassert>

#include <algorithm>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <map>
#include <unordered_map>
#include <stdexcept>

#include <yaml-cpp/yaml.h>

#include <leveldb/db.h>

#include "table_processor.hpp"
#include "table_cache_memory.hpp"

#include "table_first.h"

using table_processor_type = table_processor<first::table_t,table_cache_memory<std::unordered_map>>;
using table_cache_type = typename table_processor_type::cache_type;
using table_result_type = typename table_processor_type::result_type;

void output_results(table_result_type& results)
{
	std::cout << std::string(12, ' ');
	for (const auto& trump : suit_t::all())
	{
		std::cout << std::setw(10) << suit_t {trump};
	}
	std::cout << std::endl;

	for (const auto& side : side_t::all())
	{
		std::cout << std::setw(10) << side_t {side} << " :";
		for (const auto& trump : suit_t::all())
		{
			std::cout << std::setw(10) << static_cast<int>(results[side][trump]);
		}
		std::cout << std::endl;
	}
}

void compare_results(const YAML::Node& n, table_result_type& results)
{
	try
	{
		const auto rc {n["Result"]};
		side_t side_mismatch;
		suit_t suit_mismatch;
		if ([&]() {
				for (const auto& side : side_t::all())
				{
					const auto src {rc[side.to_string_short()]};
					for (const auto& trump : suit_t::all())
					{
						if (src[static_cast<uint8_t>(trump)].as<int>() != static_cast<int>(results[side][trump]))
						{
							side_mismatch = side;
							suit_mismatch = trump;
							return false;
							break;
						}
					}
				}
				return true;
			}())
		{
			std::cout << "Results match stored results." << std::endl;
		}
		else
		{
			std::cout << "Results DOES NOT match stored results at ["
					  << side_mismatch << ", " << suit_mismatch << "]" << std::endl;
		}
	}
	catch (const std::exception& e)
	{
		std::cout << "Results to compare not found or invalid: " << e.what() << std::endl;
	}
}

void process_table(const YAML::Node& n, table_cache_type& tc)
{
	first::table_t table {n};
	table.dump();
	if (!table.is_valid())
	{
		return;
	}

	table_processor_type tp {tc};
	auto results {tp.process_table(table)};

	double ips {static_cast<double>(tp.total_iterations()) / static_cast<double>(tp.total_duration())};
	std::cout << "Total took " << (tp.total_duration() / 1000) << " milliseconds ("
			  << tp.total_iterations() << " iteration(s); "
			  << ips << " Mips); " << tc.size() << " table(s) saved " << std::endl;

	output_results(results);
	compare_results(n, results);
}

int main(int argc, char** argv)
{
	std::unique_ptr<leveldb::DB> db;
	{
		leveldb::DB* database {nullptr};
		leveldb::DB::Open(leveldb::Options {}, "data/test_database", &database);
		db.reset(database);
	}

	if (2 > argc)
	{
		std::cout << "Argument needed." << std::endl;
		return 1;
	}

	try
	{
		table_cache_type tc {};
		std::size_t index {0};
		for (const auto& ts : YAML::LoadFile(argv[1]))
		{
			std::cout << std::string(40, '=') << std::endl;
			std::cout << "Table #" << (++index) << std::endl;

			process_table(ts, tc);

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
