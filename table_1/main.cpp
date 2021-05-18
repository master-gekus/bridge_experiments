#include "table.h"

#include <algorithm>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <map>
#include <stdexcept>

#include <yaml-cpp/yaml.h>

std::optional<move_ex_t> process_table(std::size_t indent, const table_t& t, uint64_t& iterations)
{
	if (0 == ((++iterations) % 1000000))
	{
		std::cout << std::setw(16) << iterations << std::string(16, '\b') << std::flush;
	}

	auto moves {t.available_moves()};
	for (std::size_t i = 0; i < moves.size(); ++i)
	{
		auto& m {moves[i]};
		if ((0 < i) && m.is_neighbor(moves[i - 1]))
		{
			m.add_tricks(moves[i - 1].tricks());
			continue;
		}

		//		std::cout << std::string(indent, ' ') << t.current_player() << " makes move \"" << m << "\" :" << std::endl;
		table_t nt {t};

		bool last {false};
		side_t winer;
		std::tie(last, winer) = nt.make_move(m);

		if (last)
		{
			//			std::cout << std::string(indent, ' ') << (winer.is_ns() ? "NS" : "EW") << "wins a trick." << std::endl;
			if (winer.is_ns())
			{
				m.add_tricks(1);
			}
		}

		auto res {process_table(indent + 2, nt, iterations)};
		if (res)
		{
			m.add_tricks(res->tricks());
		}
	}

	if (moves.empty())
	{
		return std::nullopt;
	}

	std::sort(moves.begin(), moves.end());
	if (t.current_player().is_ns())
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
	auto res {process_table(0, table, iterations)};
	auto dur {std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count()};
	std::cout << "took " << dur << " milliseconds (" << iterations << " iteration(s))" << std::endl;

	return std::make_pair(res->tricks(), dur);
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
