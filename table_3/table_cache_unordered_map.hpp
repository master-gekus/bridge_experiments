#ifndef TABLE_CACHE_UNORDERED_MAP_HPP
#define TABLE_CACHE_UNORDERED_MAP_HPP

#include <cstdint>

#include <map>
#include <unordered_map>

#include "moves.hpp"
#include "table_hash.hpp"

#include "table_first.h"

class table_cache_unordered_map
{
public:
	table_cache_unordered_map() = default;
	~table_cache_unordered_map() = default;

	table_cache_unordered_map(const table_cache_unordered_map&) = delete;
	table_cache_unordered_map(table_cache_unordered_map&&) = delete;
	table_cache_unordered_map& operator=(const table_cache_unordered_map&) = delete;
	table_cache_unordered_map& operator=(table_cache_unordered_map&&) = delete;

public:
	class entry_type
	{
	public:
		entry_type() = default;
		entry_type(const entry_type&) = default;
		entry_type(entry_type&&) = default;
		entry_type& operator=(const entry_type&) = default;
		entry_type& operator=(entry_type&&) = default;
		~entry_type() = default;

		inline entry_type(moves_t* entry, bool reverse, std::size_t max_tricks) noexcept
			: entry_ {entry}
			, max_tricks_ {max_tricks}
			, reverse_ {reverse}
		{
		}

	public:
		inline void update(const moves_t& moves)
		{
			if (nullptr == entry_)
			{
				return;
			}

			if (reverse_)
			{
				for (auto it {moves.rbegin()}; moves.rend() != it; --it)
				{
					entry_->push_back(move_t {it->card(), it->suit(), max_tricks_ - it->tricks()});
				}
			}
			else
			{
				(*entry_) = moves;
			}
		}

	private:
		moves_t* entry_ {nullptr};
		std::size_t max_tricks_ {0};
		bool reverse_ {false};
	};

public:
	inline std::size_t size() const
	{
		return cache_.size();
	}

	entry_type get_entry(moves_t& moves, const first::table_t& table)
	{
		moves.clear();

		const auto current_player {table.current_player()};
		const auto max_tricks {table.hand(current_player).size()};

		if ((3 > max_tricks) || (!table.is_first_move()))
		{
			return entry_type {};
		}

		using table_type = first::table_t;
		typename table_type::hash_type hash;
		table.get_hash(hash);

		const auto trump {table.trump()};
		cache_[hash].try_emplace(trump, moves);
		moves_t* entry {&(cache_[hash][trump])};

		const bool reverse {current_player.is_ns()};

		if (reverse)
		{
			for (auto it {entry->rbegin()}; entry->rend() != it; --it)
			{
				moves.push_back(move_t {it->card(), it->suit(), max_tricks - it->tricks()});
			}
		}
		else
		{
			moves = (*entry);
		}

		return entry_type {entry, reverse, max_tricks};
	}

private:
	std::unordered_map<table_hash, std::map<suit_t, moves_t>> cache_;
};

static_assert(std::is_trivially_copyable_v<table_cache_unordered_map::entry_type>);

#endif // TABLE_CACHE_UNORDERED_MAP_HPP
