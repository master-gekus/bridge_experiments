#include "table.h"

#include <cassert>

#include <algorithm>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <map>
#include <stdexcept>

#include <yaml-cpp/yaml.h>

move_ex_t process_table(std::size_t indent, const table_t& t, uint64_t& iterations,
						std::size_t max_ns_found, std::size_t max_ew_found)
{
	assert(!t.empty());

	if (0 == ((++iterations) % 1000000))
	{
		std::cout << std::setw(16) << iterations << std::string(16, '\b') << std::flush;
	}

	auto moves {t.available_moves()};
	assert(!moves.empty());

	bool is_last_move {t.is_last_move()};
	side_t current_player {t.current_player()};
	bool is_ns {current_player.is_ns()};
	std::size_t max_tricks {t.hand(current_player).size()};

	for (std::size_t i = 0; i < moves.size(); ++i)
	{
		assert(max_ns_found <= max_tricks);
		assert(max_ew_found <= max_tricks);

		auto& m {moves[i]};
		if ((0 < i) && m.is_neighbor(moves[i - 1]))
		{
			m.add_tricks(moves[i - 1].tricks());
			continue;
		}

		//		std::cout << std::string(indent, ' ') << t.current_player() << " makes move \"" << m << "\" :" << std::endl;
		table_t nt {t};

		side_t winer {nt.make_move(m)};

		if (is_last_move)
		{
			//			std::cout << std::string(indent, ' ') << (winer.is_ns() ? "NS" : "EW") << "wins a trick." << std::endl;
			if (winer.is_ns())
			{
				m.add_tricks(1);
				max_ns_found = (0 < max_ns_found) ? (max_ns_found - 1) : 0;
			}
			else {
				if (max_ns_found >= max_tricks) {
					continue;
				}
			}
		}

		if (!nt.empty())
		{
			m.add_tricks(process_table(indent + 2, nt, iterations, max_ns_found, max_ew_found).tricks());
			if (is_last_move)
			{
				if (is_ns)
				{
					max_ns_found = std::max<std::size_t>(max_ns_found, m.tricks());
				} else {

				}
			}
		}
	}

	std::sort(moves.begin(), moves.end());
	if (is_ns)
	{
		//		std::cout << std::string(indent, ' ') << "Best move for NS: " << moves.back()
		//				  << " (gives " << moves.back().tricks() << " trick(s))" << std::endl;
		return moves.back();
	}
	else
	{
		//		std::cout << std::string(indent, ' ') << "Best move for EW: " << moves.front()
		//				  << " (gives only " << moves.front().tricks() << " trick(s) for NS)" << std::endl;
		return moves.front();
	}
}

std::pair<uint8_t, uint64_t> process_table(table_t& table)
{
	std::cout << "Calculating for " << std::setw(18) << std::setiosflags(std::ios::left)
			  << (std::string {"["} + (table.current_player() - 1).to_string() + ", "
				  + table.trump().to_string() + "]")
			  << ": " << std::resetiosflags(std::ios::left) << std::flush;

	uint64_t iterations {0};
	auto start {std::chrono::steady_clock::now()};
	auto res {process_table(0, table, iterations, 0, 0)};
	auto dur {std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count()};
	double ips {(static_cast<double>(iterations) / static_cast<double>(dur)) / 1000.0};
	std::cout << "took " << dur << " milliseconds (" << iterations << " iteration(s); " << ips << " Mips)" << std::endl;

	return std::make_pair(res.tricks(), dur);
}

void process_table(const YAML::Node& n)
{
	table_t table {n};
	if (!table.is_valid())
	{
		table.dump();
		return;
	}

	std::map<side_t, std::map<suit_t, std::pair<uint8_t, uint64_t>>> results;

	for (std::size_t side = side_t::North; side <= side_t::West; ++side)
	{
		table.set_starter(side + 1);
		for (std::size_t trump = suit_t::Clubs; trump <= suit_t::Spades; ++trump)
		{
			table.set_trump(trump);
			results[side][trump] = process_table(table);
		}
		table.set_trump(suit_t::NoTrump);
		results[side][suit_t::NoTrump] = process_table(table);
	}

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
			std::cout << std::setw(10) << static_cast<int>(results[side][trump].first);
		}
		std::cout << std::setw(10) << static_cast<int>(results[side][suit_t::NoTrump].first) << std::endl;
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
				if (src[trump].as<int>() != static_cast<int>(results[side][trump].first))
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
			if (src[4].as<int>() != static_cast<int>(results[side][suit_t::NoTrump].first))
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

	//	while (true)
	//	{
	//		table.dump();
	//		uint64_t iterations {0};
	//		std::cout << "Calculating: " << std::flush;
	//		auto start {std::chrono::steady_clock::now()};
	//		auto res {process_table(0, table, iterations)};
	//		auto dur {std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count()};
	//		std::cout << "took " << dur << " milliseconds (" << iterations << " iteration(s))" << std::endl;
	//		if (!res)
	//		{
	//			break;
	//		}
	//		std::cout << "Move for " << table.current_player() << " [" << table.available_moves() << "; recomended " << (*res)
	//				  << ", gives " << res->tricks() << " for NS] : ";
	//		std::string m;
	//		std::cin >> m;
	//		table.make_move(move_t {m.c_str()});
	//	}
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
