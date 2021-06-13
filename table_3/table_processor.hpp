#ifndef TABLE_PROCESSOR_HPP
#define TABLE_PROCESSOR_HPP

#include <cassert>

#include <iostream>
#include <iomanip>

#include "table_first.h"

class table_processor
{
public:
	using table_type = table_t;
	using move_type = move_ex_t;
	using moves_type = std::vector<move_ex_t>;

	using tables_hash_type = std::map<table_t::hash_t, std::map<suit_t, std::vector<move_ex_t>>>;

public:
	inline table_processor(tables_hash_type& th) noexcept
		: m_th {th}
	{
	}

public:
	inline move_type process_table_internal(std::size_t indent, const table_type& t,
											std::size_t max_ns_found, std::size_t max_ew_found)
	{
		assert(!t.empty());

		if (0 == ((++m_iterations) % 1000000))
		{
			std::cout << std::setw(16) << m_iterations << std::string(16, '\b') << std::flush;
		}

		const bool is_last_move {t.is_last_move()};
		const side_t current_player {t.current_player()};
		const bool is_ns {current_player.is_ns()};
		const std::size_t max_tricks {t.hand(current_player).size()};
		const bool use_cache {(2 < max_tricks) && t.is_first_move()};

		moves_type moves {};
		moves_type* moves_in_cache {nullptr};

		if (use_cache)
		{
			moves_in_cache = &(m_th[t.hash()][t.trump()]);

			if (is_ns)
			{
				moves = (*moves_in_cache);
			}
			else
			{
				moves.reserve(moves_in_cache->size());
				for (auto it {moves_in_cache->crbegin()}; moves_in_cache->crend() != it; ++it)
				{
					moves.emplace_back(it->card(), it->suit(), max_tricks - it->tricks());
				}
			}
		}

		if (!moves.empty())
		{
			++m_reused;
		}
		else
		{
			moves = t.available_moves();
			assert(!moves.empty());

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

				table_type nt {t};

				side_t winer {nt.make_move(m)};

				if (is_last_move)
				{
					if (winer.is_ns())
					{
						if (max_ew_found >= max_tricks)
						{
							m.set_tricks(max_tricks);
							continue;
						}

						m.add_tricks(1);
						max_ns_found = (0 < max_ns_found) ? (max_ns_found - 1) : 0;
					}
					else
					{
						if (max_ns_found >= max_tricks)
						{
							continue;
						}

						max_ew_found = (0 < max_ew_found) ? (max_ew_found - 1) : 0;
					}
				}

				if (!nt.empty())
				{
					m.add_tricks(process_table_internal(indent + 2, nt, max_ns_found, max_ew_found).tricks());
					if (is_last_move)
					{
						if (is_ns)
						{
							max_ns_found = std::max<std::size_t>(max_ns_found, m.tricks());
						}
						else
						{
							assert(max_tricks >= m.tricks());
							max_ew_found = std::max<std::size_t>(max_ew_found, max_tricks - m.tricks());
						}
					}
				}
			}

			std::sort(moves.begin(), moves.end());

			if (nullptr != moves_in_cache)
			{
				if (is_ns)
				{
					*moves_in_cache = moves;
				}
				else
				{
					moves_in_cache->reserve(moves.size());
					for (auto it {moves.crbegin()}; moves.crend() != it; ++it)
					{
						moves_in_cache->emplace_back(it->card(), it->suit(), max_tricks - it->tricks());
					}
				}
			}
		}

		if (is_ns)
		{
			assert(!moves.back().is_max());
			return moves.back();
		}
		else
		{
			assert(!moves.front().is_max());
			return moves.front();
		}
	}

	inline uint64_t iterations() const noexcept
	{
		return m_iterations;
	}

	inline uint64_t reused() const noexcept
	{
		return m_reused;
	}

private:
	uint64_t m_iterations {0};
	uint64_t m_reused {0};
	tables_hash_type& m_th;
};

#endif // TABLE_PROCESSOR_HPP
