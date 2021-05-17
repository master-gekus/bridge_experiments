#include "table.h"

#include <algorithm>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <stdexcept>

#include <yaml-cpp/yaml.h>

std::optional<move_ex_t> process_table(std::size_t indent, const table_t& t, uint64_t& iterations)
{
	if (0 == ((++iterations) % 1000000))
	{
		std::cout << std::setw(16) << iterations << std::string(16, '\b') << std::flush;
	}

	auto moves {t.available_moves()};
	for (auto& m : moves)
	{
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

void process_table(const YAML::Node& n)
{
	table_t t {n};
	if (!t.is_valid())
	{
		t.dump();
		return;
	}

	while (true)
	{
		t.dump();
		uint64_t iterations {0};
		std::cout << "Calculating: " << std::flush;
		auto start {std::chrono::steady_clock::now()};
		auto res {process_table(0, t, iterations)};
		auto dur {std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count()};
		std::cout << "took " << dur << " milliseconds (" << iterations << " iteration(s))" << std::endl;
		if (!res)
		{
			break;
		}
		std::cout << "Move for " << t.current_player() << " [" << t.available_moves() << "; recomended " << (*res)
				  << ", gives " << res->tricks() << " for NS] : ";
		std::string m;
		std::cin >> m;
		t.make_move(move_t {m.c_str()});
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
