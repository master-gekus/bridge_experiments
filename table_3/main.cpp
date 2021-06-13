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

struct process_table_res
{
	uint8_t tricks_;
	uint64_t iterations_;
};

process_table_res process_table(table_t& table, tables_hash& th)
{
	std::cout << "Calculating for " << std::setw(18) << std::setiosflags(std::ios::left)
			  << (std::string {"["} + (table.current_player() - 1).to_string() + ", "
				  + table.trump().to_string() + "]")
			  << ": " << std::resetiosflags(std::ios::left) << std::flush;

	table_processor tp {th};

	auto start {std::chrono::steady_clock::now()};
	auto res {tp.process_table_internal(0, table, 0, 0)};
	auto dur {std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - start).count()};
	double ips {static_cast<double>(tp.iterations()) / static_cast<double>(dur)};
	std::cout << "took " << (dur / 1000) << " milliseconds (" << tp.iterations() << " iteration(s), "
			  << tp.reused() << " reused; "
			  << ips << " Mips)" << std::endl;

	return process_table_res {static_cast<uint8_t>(res.tricks()), tp.iterations()};
}

void process_table(const YAML::Node& n, tables_hash& th)
{
	table_t table {n};
	table.dump();
	if (!table.is_valid())
	{
		return;
	}

	std::map<side_t, std::map<suit_t, uint8_t>> results;
	uint64_t total_iterations {0};
	auto start {std::chrono::steady_clock::now()};

	for (std::size_t side = side_t::North; side <= side_t::West; ++side)
	{
		table.set_starter(side + 1);
		for (std::size_t trump = suit_t::Clubs; trump <= suit_t::Spades; ++trump)
		{
			table.set_trump(trump);
			const auto res {process_table(table, th)};
			total_iterations += res.iterations_;
			results[side][trump] = res.tricks_;
		}
		table.set_trump(suit_t::NoTrump);
		const auto res {process_table(table, th)};
		total_iterations += res.iterations_;
		results[side][suit_t::NoTrump] = res.tricks_;
	}

	auto dur {std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - start).count()};
	double ips {static_cast<double>(total_iterations) / static_cast<double>(dur)};
	std::cout << "Total took " << (dur / 1000) << " milliseconds (" << total_iterations << " iteration(s); "
			  << ips << " Mips); " << th.size() << " table(s) saved " << std::endl;

	// Output result table
	std::cout << std::string(12, ' ');
	for (std::size_t trump = suit_t::Clubs; trump <= suit_t::Spades; ++trump)
	{
		std::cout << std::setw(10) << suit_t {trump};
	}
	std::cout << std::setw(10) << suit_t {suit_t::NoTrump} << std::endl;

	for (std::size_t side = side_t::North; side <= side_t::West; ++side)
	{
		std::cout << std::setw(10) << side_t {side} << " :";
		for (std::size_t trump = suit_t::Clubs; trump <= suit_t::Spades; ++trump)
		{
			std::cout << std::setw(10) << static_cast<int>(results[side][trump]);
		}
		std::cout << std::setw(10) << static_cast<int>(results[side][suit_t::NoTrump]) << std::endl;
	}

	// Compare with stored results
	try
	{
		const auto rc {n["Result"]};
		bool is_match {true};
		side_t side_mismatch;
		suit_t suit_mismatch;
		for (std::size_t side = side_t::North; side <= side_t::West; ++side)
		{
			const auto src {rc[side_t {side}.to_string_short()]};
			for (std::size_t trump = suit_t::Clubs; trump <= suit_t::Spades; ++trump)
			{
				if (src[trump].as<int>() != static_cast<int>(results[side][trump]))
				{
					is_match = false;
					side_mismatch = side;
					suit_mismatch = trump;
					break;
				}
			}
			if (!is_match)
			{
				break;
			}
			if (src[4].as<int>() != static_cast<int>(results[side][suit_t::NoTrump]))
			{
				is_match = false;
				side_mismatch = side;
				suit_mismatch = suit_t::NoTrump;
				break;
			}
		}
		if (is_match)
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
