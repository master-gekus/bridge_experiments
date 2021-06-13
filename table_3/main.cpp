#include <cassert>

#include <algorithm>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <map>
#include <stdexcept>

#include <yaml-cpp/yaml.h>

#include <leveldb/db.h>

#include "table_first.h"
#include "table_processor.hpp"

using tables_hash = std::map<table_t::hash_t, std::map<suit_t, std::vector<move_ex_t>>>;

void process_table(const YAML::Node& n, tables_hash& th)
{
	table_t table {n};
	table.dump();
	if (!table.is_valid())
	{
		return;
	}

	std::map<side_t, std::map<suit_t, std::size_t>> results;
	table_processor tp {th};
	uint64_t total_iterations {0};
	auto start {std::chrono::steady_clock::now()};

	for (const auto& side : side_t::all())
	{
		table.set_starter(side + 1);
		for (const auto& trump : suit_t::all())
		{
			table.set_trump(trump);
			results[side][trump] = tp.process_table_internal(table);
			total_iterations += tp.iterations();
		}
	}

	auto dur {std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - start).count()};
	double ips {static_cast<double>(total_iterations) / static_cast<double>(dur)};
	std::cout << "Total took " << (dur / 1000) << " milliseconds (" << total_iterations << " iteration(s); "
			  << ips << " Mips); " << th.size() << " table(s) saved " << std::endl;

	// Output result table
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

	// Compare with stored results
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
		tables_hash th {};
		std::size_t index {0};
		for (const auto& ts : YAML::LoadFile(argv[1]))
		{
			std::cout << std::string(40, '=') << std::endl;
			std::cout << "Table #" << (++index) << std::endl;

			process_table(ts, th);

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
