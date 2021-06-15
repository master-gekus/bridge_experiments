#ifndef TABLE_PROCESSOR_HPP
#define TABLE_PROCESSOR_HPP

#include <cassert>

#include <algorithm>
#include <chrono>
#include <string>
#include <map>

#include "enums.hpp"

class table_processor_base
{
public:
	inline explicit table_processor_base(bool suppress_output = false)
		: m_suppress_output {suppress_output}
		, m_iterations {0}
		, m_reused {0}
	{
	}

	inline auto iterations() const noexcept
	{
		return m_iterations;
	}

	inline auto& iterations() noexcept
	{
		return m_iterations;
	}

	inline auto reused() const noexcept
	{
		return m_reused;
	}

	inline auto& reused() noexcept
	{
		return m_reused;
	}

	inline void restart_processing(const std::string& message) noexcept
	{
		m_iterations = 0;
		m_reused = 0;
		out_calculating_started(message);
	}

	void out_calculating_started(const std::string& message) const;
	void out_iterations() const;
	void out_calculating_fineshed(std::chrono::microseconds::rep microseconds_passed) const;

	template <class Rep, class Period>
	inline void out_calculating_fineshed(std::chrono::duration<Rep, Period> dur)
	{
		using namespace std::chrono;
		out_calculating_fineshed(std::chrono::duration_cast<std::chrono::microseconds>(dur).count());
	}

private:
	bool m_suppress_output {false};
	uint64_t m_iterations {0};
	uint64_t m_reused {0};
};

template <typename TableType, typename CacheType>
class table_processor : public table_processor_base
{
public:
	using table_type = TableType;
	using move_type = typename table_type::move_type;
	using moves_type = typename table_type::moves_type;
	using result_type = std::map<side_t, std::map<suit_t, uint8_t>>;
	using table_cache_type = CacheType;

public:
	inline table_processor(table_cache_type& tc, bool suppress_output = false) noexcept
		: table_processor_base {suppress_output}
		, tc_ {tc}
	{
	}

private:
	inline move_type process_table_internal(std::size_t indent, const table_type& t,
											std::size_t max_ns_found, std::size_t max_ew_found)
	{
		assert(!t.empty());

		if (0 == ((++iterations()) % 1000000))
		{
			out_iterations();
		}

		const bool is_last_move {t.is_last_move()};
		const side_t current_player {t.current_player()};
		const bool is_ns {current_player.is_ns()};
		const std::size_t max_tricks {t.max_tricks()};

		moves_type moves {};
		auto cache_entry {tc_.get_entry(moves, t)};

		if (!moves.empty())
		{
			++reused();
		}
		else
		{
			t.get_available_moves(moves);
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
			cache_entry.update(moves);
		}

		return is_ns ? moves.back() : moves.front();
	}

	uint8_t process_table_internal(table_type& table)
	{
		restart_processing(std::string {"["} + (table.current_player() - 1).to_string()
						   + ", " + table.trump().to_string() + "]");

		auto start {std::chrono::steady_clock::now()};
		auto res {process_table_internal(0, table, 0, 0)};
		out_calculating_fineshed(std::chrono::steady_clock::now() - start);

		return res.tricks();
	}

public:
	inline result_type process_table(table_type table)
	{
		using namespace std::chrono;

		result_type result;

		total_iterations_ = 0;
		auto start {steady_clock::now()};

		for (const auto& side : side_t::all())
		{
			table.set_starter(side + 1);
			for (const auto& trump : suit_t::all())
			{
				table.set_trump(trump);
				result[side][trump] = process_table_internal(table);
				total_iterations_ += iterations();
			}
		}

		total_duration_ = duration_cast<microseconds>(steady_clock::now() - start).count();

		return result;
	}

	inline auto total_iterations() const noexcept
	{
		return total_iterations_;
	}

	inline auto total_duration() const noexcept
	{
		return total_duration_;
	}

private:
	table_cache_type& tc_;
	uint64_t total_iterations_ {0};
	uint64_t total_duration_ {0};
};

#endif // TABLE_PROCESSOR_HPP
